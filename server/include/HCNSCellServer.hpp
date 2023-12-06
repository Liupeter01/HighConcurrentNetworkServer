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
#include<HCNSMsgSendTask.hpp>

template<class ClientType>
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
          void pushTemproaryClient(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient);
          void pushMessageSendingTask(
                    IN typename std::vector< std::shared_ptr<ClientType>>::iterator _clientSocket,
                    IN _PackageHeader* _header
          );

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

          /*select network model (prevent performance loss when there is no client join or leave)*/
          fd_set m_fdreadCache;                             //fd_set array backup
          bool m_isClientArrayChanged;

          /*
           * every  cell server have a temporary buffer
           * this client buffer just for temporary storage and all the client should be transfer to clientVec
           * add memory smart pointer to control memory allocation
          */
          std::mutex m_queueMutex;
          std::vector<std::shared_ptr<ClientType>> m_temporaryClientBuffer;

          /*
           * every cell server thread have one client array(permanent storage)
           * add memory smart pointer to control memory allocation
          */
          std::vector<std::shared_ptr<ClientType>> m_clientVec;

          /*cell server obj pass a client on leave signal to the tcpserver*/
          INetEvent<ClientType>* m_pNetEvent;

          /*
          * seperate msg receiving and msg sending into different threads
          * we can use HCNSTaskDispatcher to manage msg send event
          */
          HCNSTaskDispatcher* m_sendTaskDispatcher;
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
          m_interfaceFuture(_future),
          m_pNetEvent(_netEvent),
          m_sendTaskDispatcher(new HCNSTaskDispatcher),
          m_isClientArrayChanged(true)
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
                    &this->m_server_address,
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
* @update: add a method to start HCNSTaskDispatcher thread 
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSCellServer<ClientType>::startCellServer()
{
          this->m_processingThread = std::thread(
                    std::mem_fn(&HCNSCellServer<ClientType>::clientRequestProcessingThread), this, std::ref(this->m_interfaceFuture)
          );

          /*start HCNSTaskDispatcher thread*/
          this->m_sendTaskDispatcher->startCellTaskDispatch();
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
* @function: void pushTemproaryClient(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient)
* @retvalue: IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSCellServer<ClientType>::pushTemproaryClient(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient)
{
          std::lock_guard<std::mutex> _lckg(this->m_queueMutex);
          this->m_temporaryClientBuffer.push_back((*_pclient));
}

/*------------------------------------------------------------------------------------------------------
* expose an interface for producer to transfer processed data into seperated sending thread
* @function: void pushMessageSendingTask(
                              IN  typename std::vector< std::shared_ptr<ClientType>>::iterator ,
                              IN _PackageHeader* _header)

* @param: 1.[IN] typename std::vector< std::shared_ptr<ClientType>>::iterator ,
*                 2.[IN] PackageHeader* _header
* 
* @retvalue: HCNSCellTask* _task
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSCellServer<ClientType>::pushMessageSendingTask(
          IN typename std::vector< std::shared_ptr<ClientType>>::iterator  _clientSocket,
          IN _PackageHeader* _header)
{
          HCNSSendTask<ClientType>* _sendTask(new HCNSSendTask<ClientType>((*_clientSocket), _header));
          this->m_sendTaskDispatcher->addTemproaryTask(_sendTask);
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
* @function: void initServerIOMultiplexing
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSCellServer<ClientType>::initServerIOMultiplexing()
{
          /*is client array changed(including client join and leave)*/
          if (this->m_isClientArrayChanged) {
                    FD_ZERO(&this->m_fdread);                                                               //clean fd_read

                    this->m_isClientArrayChanged = false;
                    this->m_largestSocket = static_cast<SOCKET>(0);

                    /*add all the client socket in to the fd_read*/
                    for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ib++) {
                              FD_SET((*ib)->getClientSocket(), &m_fdread);
                              if (m_largestSocket < (*ib)->getClientSocket()) {
                                        m_largestSocket = (*ib)->getClientSocket();
                              }
                    }

#if _WIN32    
                    memcpy_s(
                              reinterpret_cast<void*>(&this->m_fdreadCache),
                              sizeof(fd_set),
                              reinterpret_cast<void*>(&this->m_fdread),
                              sizeof(fd_set)
                    );
#else             
                    memcpy(
                              reinterpret_cast<void*>(&this->m_fdreadCache),
                              reinterpret_cast<void*>(&this->m_fdread),
                              sizeof(fd_set)
                    );
#endif
          }
          else
          {
#if _WIN32    
                    memcpy_s(
                              reinterpret_cast<void*>(&this->m_fdread),
                              sizeof(fd_set),
                              reinterpret_cast<void*>(&this->m_fdreadCache),
                              sizeof(fd_set)
                    );

#else             
                    memcpy(
                              reinterpret_cast<void*>(&this->m_fdread),
                              reinterpret_cast<void*>(&this->m_fdreadCache),
                              sizeof(fd_set)
                    );
#endif
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
          return ::select(
                              getLargestSocketValue(),
                              &this->m_fdread,
                              &this->m_fdwrite,
                              &this->m_fdexception,
                              &this->m_timeoutSetting
        );
}

/*------------------------------------------------------------------------------------------------------
* @function:  dataProcessingLayer
* @param: [IN] typename std::vector<ClientType*>::iterator
* @description: process the request from clients
* @retvalue : bool
* @update: in order to enhance the performance of the server, we are going to
*           remove m_szRecvBuffer in cellserver and serveral memcpy functions
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
bool HCNSCellServer<ClientType>::clientDataProcessingLayer(
          IN typename std::vector<ClientType*>::iterator _clientSocket)
{
          /*add up to recv counter*/
          this->m_pNetEvent->addUpRecvCounter((*_clientSocket));

          /* We don't need to recv and store data in HCNScellServer::m_szRecvBuffer
          *  we can recv and store data in every class clientsocket directly
          */
          int  recvStatus = (*_clientSocket)->reciveDataFromClient(      //retrieve data from kernel buffer space
                    (*_clientSocket)->getMsgBufferTail(),
                    (*_clientSocket)->getBufRemainSpace()
          );

          if (recvStatus <= 0) {                                             //no data recieved!
                    std::cout << "Client's Connection Terminate<Socket =" << static_cast<int>((*_clientSocket)->getClientSocket()) << ","
                              << inet_ntoa((*_clientSocket)->getClientAddr()) << ":"
                              << (*_clientSocket)->getClientPort() << ">" << std::endl;

                    return false;
          }

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
                              this->m_pNetEvent->readMessageHeader(_clientSocket, reinterpret_cast<_PackageHeader*>(_header));
                              this->m_pNetEvent->readMessageBody(this, _clientSocket, reinterpret_cast<_PackageHeader*>(_header));
                             
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
          memset((*_clientSocket)->getMsgBufferHead(), 0, (*_clientSocket)->getBufFullSpace());
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

                              /*clients joined*/
                              this->m_isClientArrayChanged = true;
                    }

                    /*
                    * size of permanent client container should be valid
                    * suspend this thread for 1 millisecond in order to block this thread from occupying cpu cycle
                    */
                    if (this->m_clientVec.size() <= 0) {
                              std::this_thread::sleep_for(std::chrono::milliseconds(1));
                              continue;
                    }

                    this->initServerIOMultiplexing();

                    int _selectStatus = this->initServerSelectModel();

                    /*select model mulfunction*/
                    if (_selectStatus < 0) {                         
                              this->shutdownCellServer();
                              break;
                    }
                    else if(!_selectStatus){         /*cellserver hasn't receive any data*/ 
                              continue;
                    }

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
                                                  /*clients leave*/
                                                  this->m_isClientArrayChanged = true;

                                                  /*notify the tcp server to delete it from the container and dealloc it's memory*/
                                                  if (this->m_pNetEvent != nullptr) {
                                                            this->m_pNetEvent->clientOnLeave((*ib));
                                                  }

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

          /*delete HCNSTaskDispatcher and stop msg send event*/
          delete this->m_sendTaskDispatcher;

          /*reset socket*/
          this->m_server_socket = INVALID_SOCKET;
}