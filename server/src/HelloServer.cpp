#include"HelloServer.h"

_ClientAddr::_ClientAddr()
          :_ClientAddr(0, { 0 })
{
}

_ClientAddr::_ClientAddr(SOCKET _socket, sockaddr_in _addr)
          :m_clientSocket(_socket)
{
          ::memcpy(
                    reinterpret_cast<void*>(&this->m_clientAddr),
                    reinterpret_cast<const void*>(&_addr),
                    sizeof(sockaddr_in)
          );
}

_ClientAddr::~_ClientAddr()
{
          //[POTIENTAL BUG HERE!]: why _clientaddr's dtor was deployed
         // ::closesocket(this->m_clientSocket);
          this->m_clientSocket = INVALID_SOCKET;
          memset(
                    reinterpret_cast<void*>(&this->m_clientAddr),
                    0,
                    sizeof(sockaddr_in)
          );
}

HelloServer::HelloServer()
          :m_server_address{0},
          m_server_socket(INVALID_SOCKET)
{
}

HelloServer::HelloServer(unsigned short _ipPort)
          :HelloServer(INADDR_ANY,_ipPort)
{

}

/*------------------------------------------------------------------------------------------------------
* @function: HelloServer::HelloServer
* @param : 1.[IN] unsigned long _ipAddr
                    2.[IN] unsigned short _port
*------------------------------------------------------------------------------------------------------*/
HelloServer::HelloServer(
          IN unsigned long _ipAddr, 
          IN unsigned short _ipPort)
{
#if _WIN32                          //Windows Enviorment
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif
          this->initServerAddressBinding(_ipAddr, _ipPort);
}

HelloServer::~HelloServer()
{
#if _WIN32                          //Windows Enviorment
          ::shutdown(this->m_server_socket,SD_BOTH); //disconnect I/O
          WSACleanup();
          ::closesocket(this->m_server_socket);     //release socket completely!! 

#else                                   /* Unix/Linux/Macos Enviorment*/
          ::shutdown(this->m_server_socket,SHUT_RDWR);//disconnect I/O and keep recv buffer
          close(this->m_server_socket);                //release socket completely!! 

#endif
          this->m_clientVec.clear();
}

/*------------------------------------------------------------------------------------------------------
* use socket api to create a ipv4 and tcp protocol socket 
* @function: static SOCKET createServerSocket
* @param :
*                   1.[IN] int af
*                   2.[IN] int type
*                   3.[IN] int protocol
*------------------------------------------------------------------------------------------------------*/
SOCKET HelloServer::createServerSocket(
          IN int af,
          IN int type,
          IN int protocol)
{
          return  ::socket(af, type, protocol);                              //Create server socket
}

/*------------------------------------------------------------------------------------------------------
* use socket api to create a ipv4 and tcp protocol socket
* @function: static SOCKET createServerSocket
* @param :
*                   1.[IN] SOCKET  serverSocket
*                   2.[IN] int backlog
*------------------------------------------------------------------------------------------------------*/
int HelloServer::startListeningConnection(
          IN SOCKET  serverSocket,
          IN int backlog)
{
          return ::listen(serverSocket, backlog);
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
bool HelloServer::acceptClientConnection(
          IN SOCKET serverSocket,
          OUT SOCKET *clientSocket,
          OUT SOCKADDR_IN* _clientAddr)
{
#if _WIN32             //Windows Enviorment
          int addrlen = sizeof(SOCKADDR_IN);   
#else                         /* Unix/Linux/Macos Enviorment*/
          socklen_t addrlen = sizeof(SOCKADDR_IN);       
#endif  
          *clientSocket =  ::accept(
                    serverSocket, 
                    reinterpret_cast<sockaddr*>(_clientAddr),
                    &addrlen
          );
          if (*clientSocket == INVALID_SOCKET) {                 //socket error occured!
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
int HelloServer::getlargestSocketValue()
{
    SOCKET _largestSocket = this->m_server_socket;  //currently, the number of server socket is the largest
    
    /*travelersal the client socket container and try to find out the largest socket number! */
    for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ib++) {
        if(_largestSocket < ib->m_clientSocket){
            _largestSocket = ib->m_clientSocket;
        }
    }
    return ((static_cast<int>(_largestSocket))) + 1;
}

/*------------------------------------------------------------------------------------------------------
* init IO Multiplexing
* @function: void initServerIOMultiplexing
*------------------------------------------------------------------------------------------------------*/
void HelloServer::initServerIOMultiplexing()
{
          FD_ZERO(&m_fdread);                                                              //clean fd_read
          FD_ZERO(&m_fdwrite);                                                             //clean fd_write
          FD_ZERO(&m_fdexception);                                                      //clean fd_exception

          FD_SET(this->m_server_socket, &m_fdread);                           //Insert Server Socket into fd_read
          FD_SET(this->m_server_socket, &m_fdwrite);                          //Insert Server Socket into fd_write
          FD_SET(this->m_server_socket, &m_fdexception);                  //Insert Server Socket into fd_exception

          /*add all the client socket in to the fd_read*/
          for (auto ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ib++) {
                    FD_SET(ib->m_clientSocket, &m_fdread);
          }
}

/*------------------------------------------------------------------------------------------------------
* use select system call to setup model
* @function: bool initServerSelectModel
* @retvalue: bool
*------------------------------------------------------------------------------------------------------*/
bool HelloServer::initServerSelectModel()
{
          return (::select(
                    this->getlargestSocketValue(),
                    &m_fdread,
                    &m_fdwrite,
                    &m_fdexception,
                    &this->m_timeoutSetting) < 0);                  //Select Task Ended!
}

/*------------------------------------------------------------------------------------------------------
* init server ip address and port and bind it with server socket
* @function: void initServerAddressBinding
* @implementation: create server socket and bind ip address info
* @param : 1.[IN] unsigned long _ipAddr
                    2.[IN] unsigned short _port
*------------------------------------------------------------------------------------------------------*/
void HelloServer::initServerAddressBinding(
          unsigned long _ipAddr, 
          unsigned short _port)
{
          int bindStatus = -1;                                                                                   //::bind() function return value
          this->m_server_socket = createServerSocket();
          
          if (this->m_server_socket == INVALID_SOCKET) {
                    return;
          }

          this->m_server_address.sin_family = AF_INET;                                    //IPV4
          this->m_server_address.sin_port = htons(_port);                                     //Port number

#if _WIN32     //Windows Enviorment
          this->m_server_address.sin_addr.S_un.S_addr = _ipAddr;                 //IP address(Windows)   
#else               /* Unix/Linux/Macos Enviorment*/
          this->m_server_address.sin_addr.s_addr = _ipAddr;                            //IP address(LINUX) 
#endif

          bindStatus = ::bind(
                    this->m_server_socket,
                    reinterpret_cast<sockaddr*>(&this->m_server_address),
                    sizeof(SOCKADDR_IN)
          );

          if (bindStatus == SOCKET_ERROR) {
                    return;
          }
}

/*------------------------------------------------------------------------------------------------------
* init server ip address and port and bind it with server socket
* @function: void initServerAddressBinding
* @param : unsigned short _port
*------------------------------------------------------------------------------------------------------*/
void HelloServer::startServerListening(int backlog)
{
          int listenStatus = this->startListeningConnection(                                  //start listening on current server ip:port
                    this->m_server_socket, 
                    backlog
          ); 
          if (listenStatus == SOCKET_ERROR) {
                    return;
          }
}

/*------------------------------------------------------------------------------------------------------
* @function: void sendDataToClient
* @param : 
*                  1.[IN]   SOCKET& _clientSocket,
                    2.[IN]  T* _szSendBuf,
                    3.[IN] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
template<typename T>
void  HelloServer::sendDataToClient(
          IN  SOCKET& _clientSocket,
          IN  T* _szSendBuf,
          IN int _szBufferSize)
{
          ::send(
                    _clientSocket,
                    reinterpret_cast<const char*>(_szSendBuf),
                    _szBufferSize,
                    0
          );
}

/*------------------------------------------------------------------------------------------------------
* @function: void boardcastDataToAll<sockaddr_in> specialization
* @param : 
          1.[IN] SOCKET& _currSocket
          2.[IN]  T& _info
*------------------------------------------------------------------------------------------------------*/
template<typename T>
void HelloServer::boardcastDataToAll(
          IN SOCKET& _currSocket, 
          IN T& _info)
{
          if (this->m_clientVec.size() <= 1) {                                  //there is only one or no client, so return
                    return;
          }
          for (std::vector<_ClientAddr>::iterator ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ) {
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

template<> 
void HelloServer::boardcastDataToAll<sockaddr_in>(
          IN SOCKET& _currSocket,
          IN sockaddr_in& _info)
{
          if (this->m_clientVec.size() <= 1) {                                  //there is only one or no client, so return
                    return;
          }
          _BoardCast _boardPackage(inet_ntoa(_info.sin_addr), _info.sin_port);
          for (std::vector<_ClientAddr>::iterator ib = this->m_clientVec.begin(); ib != this->m_clientVec.end(); ) {
                    if (ib == this->m_clientVec.end()) {	 //judge current container status
                              break;
                    }
                    if (ib->m_clientSocket != _currSocket) { 
                              this->sendDataToClient(
                                        ib->m_clientSocket,
                                        &_boardPackage,
                                        sizeof(_boardPackage)
                              );
                    }
                    ib++;
          }
}

/*------------------------------------------------------------------------------------------------------
* @function: void reciveDataFromClient
* @param :
                    1. [IN]  SOCKET&  _clientSocket,
                    1. [OUT]  T* _szRecvBuf,
                    2. [IN] int &_szBufferSize
* @retvalue: int  
*------------------------------------------------------------------------------------------------------*/
template<typename T>
int HelloServer::reciveDataFromClient(
          IN SOCKET& _clientSocket,
          OUT T* _szRecvBuf,
          IN int _szBufferSize)
{
          return  ::recv(
                    _clientSocket,
                    reinterpret_cast<char*>(_szRecvBuf),
                    _szBufferSize,
                    0
          );
}

/*------------------------------------------------------------------------------------------------------
* @function:  dataProcessingLayer
* @param: [IN] std::vector<_ClientAddr>::iterator
* @description: process the request from clients
* @retvalue : bool
*------------------------------------------------------------------------------------------------------*/
bool HelloServer::dataProcessingLayer(IN std::vector<_ClientAddr>::iterator _clientSocket)
{
          char _buffer[256]{ 0 };
          if (!this->readMessageHeader(_clientSocket, reinterpret_cast<_PackageHeader*>(_buffer))) {       //get message header to indentify commands
                    return false;
          }
          if (!this->readMessageBody(_clientSocket, reinterpret_cast<_PackageHeader*>(_buffer))) {             //client exit
                    return false;
          }
          return true;
}

/*------------------------------------------------------------------------------------------------------
* @function:  virtual bool readMessageHeader
* @param: 
                    1.IN std::vector<_ClientAddr>::iterator _clientSocket
                    2.[IN OUT ] _PackageHeader* _header

* @description: process clients' message header
* @retvalue : bool
*------------------------------------------------------------------------------------------------------*/
bool HelloServer::readMessageHeader(
          IN std::vector<_ClientAddr>::iterator _clientSocket,
          IN OUT _PackageHeader* _header)
{
          int  recvStatus = this->reciveDataFromClient(      // record recv data status
                    _clientSocket->m_clientSocket,
                    reinterpret_cast<char*>(_header),
                    sizeof(_PackageHeader)
          );
          if (recvStatus <= 0) {                                              //Client Exit Manually 
                    std::cout << "Client's Connection Terminate<Socket =" << static_cast<int>(_clientSocket->m_clientSocket) << ","
                              << inet_ntoa(_clientSocket->m_clientAddr.sin_addr) << ":" << _clientSocket->m_clientAddr.sin_port << ">" 
                              << std::endl;
                    return false;
          }
          std::cout << "Client's Connection Request<Socket =" << static_cast<int>(_clientSocket->m_clientSocket) << ","
                    << inet_ntoa(_clientSocket->m_clientAddr.sin_addr) << ":" << _clientSocket->m_clientAddr.sin_port << "> : "
                    << "Data Length = " << _header->_packageLength << ", Request = ";
          return true;
}

/*------------------------------------------------------------------------------------------------------
* @function:  virtual void readMessageBody
* @param: 
                    1.IN std::vector<_ClientAddr>::iterator _clientSocket
                    2.[IN] _PackageHeader* _buffer
* @description:  process clients' message body
* 
* @retvalue : bool
*------------------------------------------------------------------------------------------------------*/
bool HelloServer::readMessageBody(
          IN std::vector<_ClientAddr>::iterator _clientSocket,
          IN _PackageHeader* _buffer)
{
          int recvStatus(0);
          if (_buffer->_packageCmd == CMD_LOGIN) {
                    _LoginData* loginData(reinterpret_cast<_LoginData*>(_buffer));
                    recvStatus = this->reciveDataFromClient(
                              _clientSocket->m_clientSocket,
                              reinterpret_cast<char*>(loginData) + sizeof(_PackageHeader),
                              _buffer->_packageLength - sizeof(_PackageHeader)
                    );
                    loginData->loginStatus = true;                                                                               //set login status as true

                    std::cout << "CMD_LOGIN, username = " << loginData->userName
                              << ", userpassword = " << loginData->userPassword << std::endl;

                    this->sendDataToClient(_clientSocket->m_clientSocket, loginData, sizeof(_LoginData));
          }
          else if (_buffer->_packageCmd == CMD_LOGOUT) {
                    _LogoutData *logoutData(reinterpret_cast<_LogoutData*>(_buffer));
                    recvStatus = this->reciveDataFromClient(
                              _clientSocket->m_clientSocket,
                              reinterpret_cast<char*>(logoutData) + sizeof(_PackageHeader),
                              _buffer->_packageLength - sizeof(_PackageHeader)
                    );
                    logoutData->logoutStatus = true;                                                                               //set logout status as true
                    std::cout << "CMD_LOGOUT, username = " << logoutData->userName << std::endl;

                    this->sendDataToClient(_clientSocket->m_clientSocket, &logoutData, sizeof(_LogoutData));
          }
          if (recvStatus <= 0) {                                              //Client Exit Manually
                    return false;
          }
          return true;
}

void HelloServer::serverMainFunction()
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
                              this->m_clientVec.emplace_back(_clientSocket, _clientAddress);
                              /*BoardCast Client's Connection to Other Clients (expect itself)*/
                              this->boardcastDataToAll(_clientSocket, _clientAddress);
                    }

                    //[POTIENTAL BUG HERE!]: why _clientaddr's dtor was deployed
                    for (std::vector<_ClientAddr>::iterator ib = this->m_clientVec.begin(); ib != this->m_clientVec.end();) {
                              /*
                              *Entering main logic layer std::vector<_ClientAddr>::iterator as an input to the main system
                              * retvalue: when functionlogicLayer return a value of false it means [CLIENT EXIT MANUALLY]
                              * then you have to remove it from the container
                              */
                              if (!this->dataProcessingLayer(ib)) {
                                        /*
                                        *There is a kind of sceniro which there is only one client who still remainning connection to the server
                                        */        
                                        ib = this->m_clientVec.erase(ib);                                                             //Erase Current unavailable client's socket 
                                        if (ib == this->m_clientVec.end() || this->m_clientVec.size() <= 1 ) {	 //judge current container status
                                                  break;
                                        }
                              }
                              ib++;
                    }
                     //process server's own business
          }
}