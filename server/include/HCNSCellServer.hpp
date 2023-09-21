#pragma once
#ifndef _HCNSCELLSERVER_H_
#define _HCNSCELLSERVER_H_
#include<vector>
#include<queue>
#include<future>
#include<thread>

#include<DataPackage.h>
#include<HCNSTimeStamp.h>
#include<HCNSINetEvent.hpp>
#include<HCNSMemoryManagement.hpp>

#if _WIN32
#pragma comment(lib,"HCNSMemoryPool.lib")
#endif

/*detour global memory allocation and deallocation functions*/
void* operator new(size_t _size)
{
          return MemoryManagement::getInstance().allocPool<void*>(_size);
}

void operator delete(void* _ptr)
{
          if (_ptr != nullptr) {
                    MemoryManagement::getInstance().freePool<void*>(_ptr);
          }
}

void* operator new[](size_t _size)
{
           return ::operator new(_size);
}

void operator delete[](void* _ptr)
{
           operator delete(_ptr);
}

template<typename T> T memory_alloc(size_t _size)
{
          return reinterpret_cast<T>(::malloc(_size));
}

template<typename T> void memory_free(T _ptr)
{
          ::free(reinterpret_cast<void*>(_ptr));
}

template<class ClientType = _ClientSocket>
class HCNSCellServer {
public:
          HCNSCellServer();
          HCNSCellServer(
                    IN const SOCKET& _serverSocket, 
                    IN const SOCKADDR_IN& _serverAddr,
                    IN std::shared_future<bool>& _future,
                    IN INetEvent<ClientType>* _netEvent
          );

          virtual ~HCNSCellServer();

public:
          void startCellServer();
          size_t getClientsConnectionLoad();
          void pushTemproaryClient(ClientType* _pclient);

private:
          void purgeCloseSocket(ClientType* _pclient);
          int getLargestSocketValue();
          void initServerIOMultiplexing();
          bool initServerSelectModel();
          bool clientDataProcessingLayer(
                    IN typename std::vector<ClientType*>::iterator _clientSocket
          );
          void clientRequestProcessingThread(IN std::shared_future<bool>& _future);
          void shutdownCellServer();

private:
          template<typename T> void readMessageHeader(
                    IN  typename  std::vector<ClientType*>::iterator _clientSocket,
                    IN  T* _header
          );

          template<typename T> void readMessageBody(
                    IN typename  std::vector<ClientType*>::iterator _clientSocket,
                    IN T* _header
          );

          template<typename T> void sendDataToClient(
                    IN  SOCKET& _clientSocket,
                    IN T* _szSendBuf,
                    IN int _szBufferSize
          );

          template<typename T> int reciveDataFromClient(
                    IN  SOCKET& _clientSocket,
                    OUT T* _szRecvBuf,
                    IN int _szBufferSize
          );

private:
          /*When HCNSTcpServer set promise as false, all cell server should terminate*/
          std::shared_future<bool> m_interfaceFuture;

          /*server clientRequestProcessing Thread(Consumer)*/
          std::thread m_processingThread;

          /*server socket info*/
          SOCKET m_server_socket;                           //server listening socket
          sockaddr_in m_server_address;

          /*select network model*/
          fd_set m_fdread;
          fd_set m_fdwrite;
          fd_set m_fdexception;
          timeval m_timeoutSetting{ 0/*0 s*/, 0 /*0 ms*/ };
          SOCKET m_largestSocket;

          /*
          * additional buffer space for clientDataProcessingLayer()
          * server recive buffer(retrieve much data as possible from kernel)
          */
          const unsigned int m_szRecvBufSize = 4096;
          std::shared_ptr<char> m_szRecvBuffer; 

          /*
           * every  cell server have a temporary buffer
           * this client buffer just for temporary storage and all the client should be transfer to clientVec
          */
          std::mutex m_queueMutex;
          typename std::vector<ClientType*> m_temporaryClientBuffer;

          /*every cell server thread have one client array(permanent storage)*/
          typename std::vector<ClientType*> m_clientVec;

          /*cell server obj pass a client on leave signal to the tcpserver*/
          typename INetEvent<ClientType>* m_pNetEvent;
};
#endif

template<class ClientType>
HCNSCellServer<ClientType>::HCNSCellServer()
          :m_server_address{ 0 },
          m_server_socket(INVALID_SOCKET),
          m_pNetEvent(nullptr)
{
}

template<class ClientType>
HCNSCellServer<ClientType>::HCNSCellServer(
          IN const SOCKET& _serverSocket,
          IN const SOCKADDR_IN& _serverAddr,
          IN std::shared_future<bool>& _future,
          IN INetEvent<ClientType>* _netEvent)
          : m_server_socket(_serverSocket),
          m_szRecvBuffer(new char[m_szRecvBufSize]),
          m_interfaceFuture(_future),
          m_pNetEvent(_netEvent)
{
#if _WIN32    
          memcpy_s(
                    &this->m_server_address,
                    sizeof(this->m_server_address),
                    &_serverAddr,
                    sizeof(SOCKADDR_IN)
          );

#else             
          memcpy(
                    this->m_server_address,
                    &_serverAddr,
                    sizeof(SOCKADDR_IN)
          );

#endif
}

template<class ClientType>
HCNSCellServer<ClientType>::~HCNSCellServer()
{
          this->shutdownCellServer();
}

/*------------------------------------------------------------------------------------------------------
* start cell server and create a thread for clientRequestProcessing
* @function: void startCellServer
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSCellServer<ClientType>::startCellServer()
{
          this->m_processingThread = std::thread(
                    std::mem_fn(&HCNSCellServer<ClientType>::clientRequestProcessingThread), this, std::ref(this->m_interfaceFuture)
          );
}

/*------------------------------------------------------------------------------------------------------
* how many clients are still inside the connection container(including those inside the permanent and temproary queue)
* @function: size_t getClientsConnectionLoad
* @retvalue: size_t 
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
size_t HCNSCellServer<ClientType>::getClientsConnectionLoad()
{
          /*including those inside the permanent and temproary queue*/
          return this->m_clientVec.size() + this->m_temporaryClientBuffer.size();
}

/*------------------------------------------------------------------------------------------------------
* expose an interface for consumer to insert client connection into the temproary container
* @function: void pushTemproaryClient(ClientType* _pclient)
* @retvalue: ClientType* _pclient
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSCellServer<ClientType>::pushTemproaryClient(ClientType* _pclient)
{
          std::lock_guard<std::mutex> _lckg(this->m_queueMutex);
          this->m_temporaryClientBuffer.push_back(_pclient);
}

/*------------------------------------------------------------------------------------------------------
* shutdown and terminate network connection 
* @function: void pushTemproaryClient(ClientType* _pclient)
* @retvalue: ClientType* _pclient
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSCellServer<ClientType>::purgeCloseSocket(ClientType* _pclient)
{
#if _WIN32
          ::shutdown(_pclient->getClientSocket(), SD_BOTH);                        //disconnect I/O
          ::closesocket(_pclient->getClientSocket());                                        //release socket completely!! 
#else 
          ::shutdown(_pclient->getClientSocket(), SHUT_RDWR);                 //disconnect I/O and keep recv buffer
          ::close(_pclient->getClientSocket());                                                   //release socket completely!! 
#endif
          delete _pclient;
}

/*------------------------------------------------------------------------------------------------------
* get largest socket value
* @function: int getLargestSocketValue()
* @retvalue: int 
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
int HCNSCellServer<ClientType>::getLargestSocketValue()
{
          return static_cast<int>(this->m_largestSocket) + 1;
}

/*------------------------------------------------------------------------------------------------------
* init IO Multiplexing for clientRequestProcessing
* @function: SOCKET initServerIOMultiplexing
* @retvalue: SOCKET
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSCellServer<ClientType>::initServerIOMultiplexing()
{
          FD_ZERO(&this->m_fdread);                                                               //clean fd_read
          m_largestSocket = static_cast<SOCKET>(0);

          /*add all the client socket in to the fd_read*/
          for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ib++) {
                    FD_SET((*ib)->getClientSocket(), &m_fdread);
                    if (m_largestSocket < (*ib)->getClientSocket()) {
                              m_largestSocket = (*ib)->getClientSocket();
                    }
          }
}

/*------------------------------------------------------------------------------------------------------
* use select system call to setup model
* @function: bool initServerSelectModel()
* @retvalue: bool
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
bool HCNSCellServer<ClientType>::initServerSelectModel()
{
          return (
                    ::select(
                              getLargestSocketValue(),
                              &this->m_fdread,
                              &this->m_fdwrite,
                              &this->m_fdexception,
                              &this->m_timeoutSetting
                    ) < 0
        );
}

/*------------------------------------------------------------------------------------------------------
* @function:  dataProcessingLayer
* @param: [IN] typename std::vector<ClientType*>::iterator
* @description: process the request from clients
* @retvalue : bool
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
bool HCNSCellServer<ClientType>::clientDataProcessingLayer(
          IN typename std::vector<ClientType*>::iterator _clientSocket)
{
          int  recvStatus = this->reciveDataFromClient(      //retrieve data from kernel buffer space
                    (*_clientSocket)->getClientSocket(),
                    this->m_szRecvBuffer.get(),
                    this->m_szRecvBufSize
          );

          if (recvStatus <= 0) {                                             //no data recieved!
                    std::cout << "Client's Connection Terminate<Socket =" << static_cast<int>((*_clientSocket)->getClientSocket()) << ","
                              << inet_ntoa((*_clientSocket)->getClientAddr()) << ":"
                              << (*_clientSocket)->getClientPort() << ">" << std::endl;

                    return false;
          }

          /*transmit m_szRecvBuffer to m_szMsgBuffer*/
#if _WIN32     //Windows Enviorment
          memcpy_s(
                    (*_clientSocket)->getMsgBufferTail(),
                    (*_clientSocket)->getBufRemainSpace(),
                    this->m_szRecvBuffer.get(),
                    recvStatus
          );

#else               /* Unix/Linux/Macos Enviorment*/
          memcpy(
                    (*_clientSocket)->getMsgBufferTail(),
                    this->m_szRecvBuffer.get(),
                    recvStatus
          );

#endif

          (*_clientSocket)->increaseMsgBufferPos(recvStatus);                //update the pointer pos of message array

          /* judge whether the length of the data in message buffer is bigger than the sizeof(_PackageHeader) */
          while ((*_clientSocket)->getMsgPtrPos() >= sizeof(_PackageHeader))
          {
                    _PackageHeader* _header(reinterpret_cast<_PackageHeader*>(
                              (*_clientSocket)->getMsgBufferHead()
                    ));

                    /*the size of current message in szMsgBuffer is bigger than the package length(_header->_packageLength)*/
                    if (_header->_packageLength <= (*_clientSocket)->getMsgPtrPos())
                    {             
                              /*add up to HCNSTcpServer package counter*/
                              this->m_pNetEvent->addUpPackageCounter();

                              //get message header to indentify commands    
                              //this->readMessageHeader(_clientSocket, reinterpret_cast<_PackageHeader*>(_header));
                              //this->readMessageBody(_clientSocket, reinterpret_cast<_PackageHeader*>(_header));

                              /* 
                              * delete this message package and modify the array
                              */

#if _WIN32     //Windows Enviorment
                              memcpy_s(
                                        (*_clientSocket)->getMsgBufferHead(),                                                     //the head of message buffer array
                                        (*_clientSocket)->getBufFullSpace(),
                                        (*_clientSocket)->getMsgBufferHead() + _header->_packageLength,       //the next serveral potential packages position
                                        (*_clientSocket)->getMsgPtrPos() - _header->_packageLength                  //the size of next serveral potential package
                              );

#else               /* Unix/Linux/Macos Enviorment*/
                              memcpy(
                                        (*_clientSocket)->getMsgBufferHead(),                                                      //the head of message buffer array
                                        (*_clientSocket)->getMsgBufferHead() + _header->_packageLength,       //the next serveral potential packages position
                                        (*_clientSocket)->getMsgPtrPos() - _header->_packageLength                  //the size of next serveral potential package
                              );
#endif
                              (*_clientSocket)->decreaseMsgBufferPos(_header->_packageLength);       //update the pointer pos of message array                       //recalculate the size of the rest of the array
                    }
                    else
                    {
                              /*
                               * the size of current message in szMsgBuffer is insufficent !
                               * even can not satisfied the basic requirment of sizeof(_PackageHeader)
                               */
                              break;
                    }
          }

          /*clean the recv buffer*/
          memset(this->m_szRecvBuffer.get(), 0, this->m_szRecvBufSize);
          return true;
}

/*------------------------------------------------------------------------------------------------------
* processing clients request(Consumer Thread)
* @function: void clientRequestProcessingThread(IN std::shared_future<bool>& _future)
* @param: IN std::shared_future<bool>& _future
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSCellServer<ClientType>::clientRequestProcessingThread(
          IN std::shared_future<bool>& _future)
{
          while (true) 
          {
                    if (_future.wait_for(std::chrono::microseconds(1)) == std::future_status::ready) {
                              if (!_future.get()) {
                                        break;
                              }
                    }
                    /*size of temporary buffer is valid*/
                    if (this->m_temporaryClientBuffer.size()) 
                    {
                              std::lock_guard<std::mutex> _lckg(this->m_queueMutex);
                              for (auto ib = this->m_temporaryClientBuffer.begin(); ib != this->m_temporaryClientBuffer.end(); ++ib) {
                                        this->m_clientVec.push_back(*ib);
                              }
                              this->m_temporaryClientBuffer.clear();
                    }

                    /*
                    * size of permanent client container should be valid
                    * otherwise, sleep for 1 microsecond and continue this loop
                    */
                    if (this->m_clientVec.size() <= 0) {
                              std::this_thread::sleep_for(std::chrono::microseconds(1));
                              continue;
                    }

                    this->initServerIOMultiplexing();
                    if (this->initServerSelectModel()) {
                              break;
                    }

                    ////[POTIENTAL BUG HERE!]: why _clientaddr's dtor was deployed
                    for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end();)
                    {

                              /*Detect client message input signal*/
                              if (FD_ISSET((*ib)->getClientSocket(), &m_fdread))
                              {

                                        /*
                                        *Entering main logic layer std::vector<_ClientSocket>::iterator as an input to the main system
                                        * retvalue: when functionlogicLayer return a value of false it means [CLIENT EXIT MANUALLY]
                                        * then you have to remove it from the container
                                        */
                                        if (!this->clientDataProcessingLayer(ib))
                                        {
                                                  /*notify the tcp server to delete it from the container and dealloc it's memory*/
                                                  this->m_pNetEvent->clientOnLeave((*ib));

                                                  /*modify code from just delete (*ib) to shutdown socket first and then delete*/
                                                  this->purgeCloseSocket((*ib));

                                                  /* erase Current unavailable client's socket(no longer needs to dealloc it's memory)*/
                                                  ib = this->m_clientVec.erase(ib);

                                                  /*
                                                    * There is a kind of sceniro when delete socket obj is completed
                                                    * and there is only one client still remainning connection to the server
                                                    */
                                                  if (ib == this->m_clientVec.end() || this->m_clientVec.size() <= 1) {
                                                            break;
                                                  }
                                        }
                              }
                              ib++;
                    }
          }
}

/*------------------------------------------------------------------------------------------------------
* @function:  void readMessageHeader
* @param:  1.[IN] typename std::vector<ClientType*>::iterator _clientSocket
                    2.[IN ] T* _header

* @description: process clients' message header
*------------------------------------------------------------------------------------------------------*/
template<class ClientType> template<typename T>
void HCNSCellServer<ClientType>::readMessageHeader(
          IN typename std::vector<ClientType*>::iterator _clientSocket,
          IN T* _header)
{
          std::cout << "Client's Connection Request<Socket =" << static_cast<int>((*_clientSocket)->getClientSocket()) << ","
                    << inet_ntoa((*_clientSocket)->getClientAddr()) << ":" << (*_clientSocket)->getClientPort() << "> : "
                    << "Data Length = " << _header->_packageLength << ", Request = ";

          if (_header->_packageCmd == CMD_LOGIN) {
                    std::cout << "CMD_LOGIN, ";
          }
          else if (_header->_packageCmd == CMD_LOGOUT) {
                    std::cout << "CMD_LOGOUT, ";
          }
}

/*------------------------------------------------------------------------------------------------------
* @function:  virtual void readMessageBody
* @param:  1.[IN] typename std::vector<ClientType*>::iterator _clientSocket
                    2.[IN] _PackageHeader* _buffer

* @description:  process clients' message body
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>  template<typename T>
void HCNSCellServer<ClientType>::readMessageBody(
          IN typename std::vector<ClientType*>::iterator _clientSocket,
          IN  T* _header)
{
          if (_header->_packageCmd == CMD_LOGIN) {
                    _LoginData* loginData(reinterpret_cast<_LoginData*>(_header));
                    loginData->loginStatus = true;                                                                               //set login status as true
                    std::cout << "username = " << loginData->userName
                              << ", userpassword = " << loginData->userPassword << std::endl;

                    this->sendDataToClient((*_clientSocket)->getClientSocket(), loginData, sizeof(_LoginData));
          }
          else if (_header->_packageCmd == CMD_LOGOUT) {
                    _LogoutData* logoutData(reinterpret_cast<_LogoutData*>(_header));
                    logoutData->logoutStatus = true;                                                                               //set logout status as true
                    std::cout << "username = " << logoutData->userName << std::endl;

                    this->sendDataToClient((*_clientSocket)->getClientSocket(), logoutData, sizeof(_LogoutData));
          }
          else {
                    _PackageHeader _error(sizeof(_PackageHeader), CMD_ERROR);
                    this->sendDataToClient((*_clientSocket)->getClientSocket(), &_error, sizeof(_PackageHeader));
          }
}

/*------------------------------------------------------------------------------------------------------
* @function: void sendDataToClient
* @param : 1.[IN]   SOCKET& _clientSocket,
                    2.[IN]  T* _szSendBuf,
                    3.[IN] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
template<class ClientType> template<typename T>
void HCNSCellServer<ClientType>::sendDataToClient(
          IN  SOCKET& _clientSocket,
          IN T* _szSendBuf,
          IN int _szBufferSize)
{
          ::send(_clientSocket, reinterpret_cast<const char*>(_szSendBuf), _szBufferSize, 0);
}

/*------------------------------------------------------------------------------------------------------
* @function: void reciveDataFromClient
* @param : 1. [IN]  SOCKET&  _clientSocket,
                    2. [OUT]  T* _szRecvBuf,
                    3. [IN] int &_szBufferSize
* @retvalue: int
*------------------------------------------------------------------------------------------------------*/
template<class ClientType> template<typename T>
int HCNSCellServer<ClientType>::reciveDataFromClient(
          IN  SOCKET& _clientSocket,
          OUT T* _szRecvBuf,
          IN int _szBufferSize)
{
          return  ::recv(_clientSocket, reinterpret_cast<char*>(_szRecvBuf), _szBufferSize, 0);
}

/*------------------------------------------------------------------------------------------------------
* close and erase all the clients' connection and cleanup network setup
* @function: void shutdownServer
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSCellServer<ClientType>::shutdownCellServer()
{
          /*-------------------------------------------------------------------------
          * close all the clients' connection in this cell server
          -------------------------------------------------------------------------*/
          for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ib++) 
          {
                    this->m_pNetEvent->clientOnLeave((*ib));
                    this->purgeCloseSocket((*ib));
          }
          this->m_clientVec.clear();                                            //clean the std::vector container

          if (this->m_processingThread.joinable()) {
                    this->m_processingThread.join();
          }

          /*reset socket*/
          this->m_server_socket = INVALID_SOCKET;
}