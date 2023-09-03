#pragma once
#ifndef _HELLOSERVER_H_
#define _HELLOSERVER_H_
#include<DataPackage.h>
#include<ClientSocket.h>
#include<cassert>
#include<vector>
#include<future>
#include<thread>

template<class ClientType =  _ClientSocket> 
class HelloServer{
public:
          HelloServer()
                    :m_server_address{ 0 },
                    m_server_socket(INVALID_SOCKET)
          { }

          HelloServer(IN unsigned short _ipPort)
                    :HelloServer<ClientType>(INADDR_ANY, _ipPort)
          {}

          HelloServer(IN unsigned long _ipAddr, IN unsigned short _ipPort);
          virtual ~HelloServer();

public:
          /*------------------------------------------------------------------------------------------------------
          * use socket api to create a ipv4 and tcp protocol socket
          * @function: static SOCKET createServerSocket
          * @param :  1.[IN] int af
          *                   2.[IN] int type
          *                   3.[IN] int protocol
          *------------------------------------------------------------------------------------------------------*/
          static SOCKET createServerSocket(IN int af = AF_INET, IN int type = SOCK_STREAM, IN int protocol = IPPROTO_TCP) {
                    return ::socket(af, type, protocol);                              //Create server socket
          }

          /*------------------------------------------------------------------------------------------------------
          * use socket api to create a ipv4 and tcp protocol socket
          * @function: static SOCKET createServerSocket
          * @param :  1.[IN] SOCKET  serverSocket
          *                   2.[IN] int backlog
          *------------------------------------------------------------------------------------------------------*/
          static int startListeningConnection(IN SOCKET serverSocket, IN int backlog = SOMAXCONN) {
                    return ::listen(serverSocket, backlog);
          }

          /*------------------------------------------------------------------------------------------------------
          * start server listening
          * @function:  int startServerListening
          * @param : int backlog
          * @retvalue : int
          *------------------------------------------------------------------------------------------------------*/
          int startServerListening(int backlog = SOMAXCONN) {
                    return startListeningConnection(this->m_server_socket, backlog);
          }

          static bool acceptClientConnection(
                    IN SOCKET serverSocket,
                    OUT SOCKET* clientSocket,
                    OUT SOCKADDR_IN* _clientAddr
          );

          void serverMainFunction();

private:
          void initServerAddressBinding(
                    unsigned long _ipAddr,
                    unsigned short _port
          );

          void initServerIOMultiplexing();
          bool initServerSelectModel();
          bool dataProcessingLayer(IN typename  std::vector<ClientType*>::iterator  _clientSocket);
          int getlargestSocketValue();

          template<typename T> void readMessageHeader(
                    IN  typename  std::vector<ClientType*>::iterator _clientSocket,
                    IN  T* _header
          );

          template<typename T> void readMessageBody(
                    IN typename  std::vector<ClientType*>::iterator _clientSocket,
                    IN T* _header
          );

          template<typename T> void boardcastDataToAll(
                    IN SOCKET& _currSocket,
                    IN T& _info
          );

private:
          /*------------------------------------------------------------------------------------------------------
          * @function: void sendDataToClient
          * @param : 1.[IN]   SOCKET& _clientSocket,
                              2.[IN]  T* _szSendBuf,
                              3.[IN] int _szBufferSize
          *------------------------------------------------------------------------------------------------------*/
          template<typename T> 
          void sendDataToClient(IN  SOCKET& _clientSocket, IN T* _szSendBuf, IN int _szBufferSize) {
                    ::send(_clientSocket, reinterpret_cast<const char*>(_szSendBuf), _szBufferSize, 0);
          }

          /*------------------------------------------------------------------------------------------------------
          * @function: void reciveDataFromClient
          * @param : 1. [IN]  SOCKET&  _clientSocket,
                              2. [OUT]  T* _szRecvBuf,
                              3. [IN] int &_szBufferSize
          * @retvalue: int
          *------------------------------------------------------------------------------------------------------*/
          template<typename T> 
          int reciveDataFromClient(IN  SOCKET& _clientSocket, OUT T* _szRecvBuf, IN int _szBufferSize) {
                    return  ::recv(_clientSocket, reinterpret_cast<char*>(_szRecvBuf), _szBufferSize, 0);
          }

private:
          /*select network model*/
          SOCKET m_server_socket;                           //server listening socket
          sockaddr_in m_server_address;
          fd_set m_fdread;
          fd_set m_fdwrite; 
          fd_set m_fdexception;
          timeval m_timeoutSetting{ 0/*0 s*/, 0 /*0 ms*/ };

          /*clients info*/
          typename std::vector<ClientType*> m_clientVec;

          /*server 10KB memory buffer*/
          const unsigned int m_szRecvBufSize = 1024 * 10;                       //10KB
          std::shared_ptr<char> m_szRecvBuffer;                                       //server recive buffer(retrieve much data as possible from kernel)

#if _WIN32 
          WSADATA m_wsadata;
#endif // _WINDOWS 
};
#endif

/*------------------------------------------------------------------------------------------------------
* @function: HelloServer::HelloServer
* @param : 1.[IN] unsigned long _ipAddr
                    2.[IN] unsigned short _port
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
HelloServer<ClientType>::HelloServer(
          IN unsigned long _ipAddr,
          IN unsigned short _ipPort)
          : m_szRecvBuffer(new char[m_szRecvBufSize] {0})
{
#if _WIN32                          //Windows Enviorment
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif
          this->initServerAddressBinding(_ipAddr, _ipPort);
}

template<class ClientType>
HelloServer<ClientType>::~HelloServer()
{
          /*add all the client socket in to the fd_read*/
          for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ib++) {
#if _WIN32                          //Windows Enviorment
                    ::shutdown((*ib)->getClientSocket(), SD_BOTH); //disconnect I/O
                    ::closesocket((*ib)->getClientSocket());     //release socket completely!! 

#else                                   /* Unix/Linux/Macos Enviorment*/
                    ::shutdown((*ib)->getClientSocket(), SHUT_RDWR);//disconnect I/O and keep recv buffer
                    ::close((*ib)->getClientSocket());                //release socket completely!! 
#endif
                    delete (*ib);
          }
          this->m_clientVec.clear();                                            //clean the std::vector container

#if _WIN32                          //Windows Enviorment
          ::shutdown(this->m_server_socket, SD_BOTH); //disconnect I/O
          ::closesocket(this->m_server_socket);     //release socket completely!! 
          WSACleanup();
#else                                   /* Unix/Linux/Macos Enviorment*/
          ::shutdown(this->m_server_socket, SHUT_RDWR);//disconnect I/O and keep recv buffer
          close(this->m_server_socket);                //release socket completely!! 
#endif
}

/*------------------------------------------------------------------------------------------------------
* use accept function to accept connection which come from client
* @function: static bool acceptClientConnection
* @param :
*                   1.[IN] SOCKET  serverSocket
*                   3.[OUT] SOCKET *clientSocket,
*                   2.[OUT] SOCKADDR_IN* _clientAddr
*
* @retvalue: bool
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
bool HelloServer<ClientType>::acceptClientConnection(
          IN SOCKET serverSocket,
          OUT SOCKET* clientSocket,
          OUT SOCKADDR_IN* _clientAddr)
{
#if _WIN32             //Windows Enviorment
          int addrlen = sizeof(SOCKADDR_IN);
#else                         /* Unix/Linux/Macos Enviorment*/
          socklen_t addrlen = sizeof(SOCKADDR_IN);
#endif  
          if ((*clientSocket = ::accept(serverSocket, reinterpret_cast<sockaddr*>(_clientAddr), &addrlen)) == INVALID_SOCKET) {                 //socket error occured!
                    return false;
          }
          std::cout << "Accept Client's Connection<Socket =" << static_cast<int>(*clientSocket) << ","
                    << inet_ntoa(_clientAddr->sin_addr) << ":" << _clientAddr->sin_port << ">" << std::endl;
          return true;
}

/*------------------------------------------------------------------------------------------------------
* select function needs the larest SOCKET number and add up one
* @function: int getlargestSocketValue
* @retvalue: int-> as the larest number
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
int HelloServer<ClientType>::getlargestSocketValue()
{
          SOCKET _largestSocket = this->m_server_socket;  //currently, the number of server socket is the largest

          /*travelersal the client socket container and try to find out the largest socket number! */
          for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ib++) {
                    if (_largestSocket < (*ib)->getClientSocket()) {
                              _largestSocket = (*ib)->getClientSocket();
                    }
          }
          return ((static_cast<int>(_largestSocket))) + 1;
}

/*------------------------------------------------------------------------------------------------------
* init IO Multiplexing
* @function: void initServerIOMultiplexing
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HelloServer<ClientType>::initServerIOMultiplexing()
{
          FD_ZERO(&this->m_fdread);                                                               //clean fd_read
          FD_ZERO(&this->m_fdwrite);                                                             //clean fd_write
          FD_ZERO(&this->m_fdexception);                                                      //clean fd_exception

          FD_SET(this->m_server_socket, &this->m_fdread);                           //Insert Server Socket into fd_read
          FD_SET(this->m_server_socket, &this->m_fdwrite);                          //Insert Server Socket into fd_write
          FD_SET(this->m_server_socket, &this->m_fdexception);                  //Insert Server Socket into fd_exception

          /*add all the client socket in to the fd_read*/
          for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ib++) {
                    FD_SET((*ib)->getClientSocket(), &m_fdread);
          }
}

/*------------------------------------------------------------------------------------------------------
* use select system call to setup model
* @function: bool initServerSelectModel
* @retvalue: bool
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
bool HelloServer<ClientType>::initServerSelectModel()
{
          return (::select(
                    this->getlargestSocketValue(),
                    &this->m_fdread,
                    &this->m_fdwrite,
                    &this->m_fdexception,
                    &this->m_timeoutSetting) < 0);                  //Select Task Ended!
}

/*------------------------------------------------------------------------------------------------------
* init server ip address and port and bind it with server socket
* @function: void initServerAddressBinding
* @implementation: create server socket and bind ip address info
* @param : 1.[IN] unsigned long _ipAddr
                    2.[IN] unsigned short _port
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HelloServer<ClientType>::initServerAddressBinding(
          unsigned long _ipAddr,
          unsigned short _port)
{
          if ((this->m_server_socket = createServerSocket()) == INVALID_SOCKET) {
                    return;
          }
          this->m_server_address.sin_family = AF_INET;                                    //IPV4
          this->m_server_address.sin_port = htons(_port);                                     //Port number

#if _WIN32     //Windows Enviorment
          this->m_server_address.sin_addr.S_un.S_addr = _ipAddr;                 //IP address(Windows)   
#else               /* Unix/Linux/Macos Enviorment*/
          this->m_server_address.sin_addr.s_addr = _ipAddr;                            //IP address(LINUX) 
#endif

          if (::bind(this->m_server_socket, reinterpret_cast<sockaddr*>(&this->m_server_address), sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
                    return;
          }
}

/*------------------------------------------------------------------------------------------------------
* @function: void boardcastDataToAll<sockaddr_in> specialization
* @param :
          1.[IN] SOCKET& _currSocket
          2.[IN]  T& _info
*------------------------------------------------------------------------------------------------------*/
template<class ClientType> template<typename T>
void HelloServer<ClientType>::boardcastDataToAll(
          IN SOCKET& _currSocket,
          IN T& _info)
{
          if (this->m_clientVec.size() <= 1) {                                  //there is only one or no client, so return
                    return;
          }
          for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ) {
                    if (ib == this->m_clientVec.end()) {	 //judge current container status
                              break;
                    }
                    if (ib->m_clientSocket != _currSocket) {
                              this->sendDataToClient(
                                        ib->m_clientSocket,
                                        &_info,
                                        sizeof(T)
                              );
                    }
                    ib++;
          }
}

template<> template<>
void HelloServer<>::boardcastDataToAll<sockaddr_in>(
          IN SOCKET& _currSocket,
          IN sockaddr_in& _info)
{
          if (this->m_clientVec.size() <= 1) {                                  //there is only one or no client, so return
                    return;
          }
          _BoardCast _boardPackage(inet_ntoa(_info.sin_addr), _info.sin_port);
          for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ) {
                    if (ib == this->m_clientVec.end()) {	 //judge current container status
                              break;
                    }
                    if ((*ib)->getClientSocket() != _currSocket) {
                              this->sendDataToClient(
                                        (*ib)->getClientSocket(),
                                        &_boardPackage,
                                        sizeof(_boardPackage)
                              );
                    }
                    ib++;
          }
}

/*------------------------------------------------------------------------------------------------------
* @function:  void readMessageHeader
* @param:
                    1.[IN] typename std::vector<ClientType*>::iterator _clientSocket
                    2.[IN ] T* _header

* @description: process clients' message header
*------------------------------------------------------------------------------------------------------*/
template<class ClientType> template<typename T>
void HelloServer<ClientType>::readMessageHeader(
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
* @param:
                    1.[IN] typename std::vector<ClientType*>::iterator _clientSocket
                    2.[IN] _PackageHeader* _buffer

* @description:  process clients' message body
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>  template<typename T>
void HelloServer<ClientType>::readMessageBody(
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
* @function:  dataProcessingLayer
* @param: [IN] typename std::vector<ClientType*>::iterator
* @description: process the request from clients
* @retvalue : bool
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
bool HelloServer<ClientType>::dataProcessingLayer(
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
                    if (_header->_packageLength <= (*_clientSocket)->getMsgPtrPos()) {

                              //get message header to indentify commands    
                              this->readMessageHeader(_clientSocket, reinterpret_cast<_PackageHeader*>(_header));
                              this->readMessageBody(_clientSocket, reinterpret_cast<_PackageHeader*>(_header));

                              /* delete this message package and modify the array*/
#if _WIN32     //Windows Enviorment
                              memcpy_s(
                                        (*_clientSocket)->getMsgBufferHead(),                                            //the head of message buffer array
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

template<class ClientType>
void HelloServer<ClientType>::serverMainFunction()
{
          while (1) {
                    this->initServerIOMultiplexing();
                    if (this->initServerSelectModel()) {
                              break;
                    }

#if _WIN32      
                    /*SERVER ACCELERATION PROPOSESD(Windows Enviorment only!)*/
                    if (!this->m_fdread.fd_count) {                                         //in fd_read array, no socket has been found!!               
                    }
#endif

                    if (FD_ISSET(this->m_server_socket, &m_fdread)) {    //Detect client message input signal
                              SOCKET _clientSocket;                                       //the temp variable to record client's socket
                              sockaddr_in _clientAddress;                                 //the temp variable to record client's address                                     
                              FD_CLR(this->m_server_socket, &m_fdread);      //delete client message signal

                              this->acceptClientConnection(                               //
                                        this->m_server_socket,
                                        &_clientSocket,
                                        &_clientAddress
                              );
                              this->m_clientVec.push_back(new ClientType(_clientSocket, _clientAddress));
                              /*BoardCast Client's Connection to Other Clients (expect itself)*/
                              this->boardcastDataToAll(_clientSocket, _clientAddress);
                    }

                    //[POTIENTAL BUG HERE!]: why _clientaddr's dtor was deployed
                    for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end();) {

                              /*Detect client message input signal*/
                              if (FD_ISSET((*ib)->getClientSocket(), &m_fdread)) {
                                        /*
                                        *Entering main logic layer std::vector<_ClientSocket>::iterator as an input to the main system
                                        * retvalue: when functionlogicLayer return a value of false it means [CLIENT EXIT MANUALLY]
                                        * then you have to remove it from the container
                                        */
                                        if (!this->dataProcessingLayer(ib)) {
                                                  /*
                                                  *There is a kind of sceniro which there is only one client who still remainning connection to the server
                                                  */
                                                  delete (*ib);                                                                                              //delete _ClientSocket obj
                                                  ib = this->m_clientVec.erase(ib);                                                             //Erase Current unavailable client's socket 
                                                  if (ib == this->m_clientVec.end() || this->m_clientVec.size() <= 1) {	 //judge current container status
                                                            break;
                                                  }
                                        }

                              }
                              ib++;
                    }
                    //process server's own business
          }
}