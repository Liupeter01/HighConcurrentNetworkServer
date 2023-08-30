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
* @function��HelloServer::HelloServer
* @param : 1.[IN] unsigned long _ipAddr
                    2.[IN] unsigned short _port
*------------------------------------------------------------------------------------------------------*/
HelloServer::HelloServer(
          IN unsigned long _ipAddr, 
          IN unsigned short _ipPort)
{
#ifdef _WINDOWS
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif // _WINDOWS
          this->initServerAddressBinding(_ipAddr, _ipPort);
}

HelloServer::~HelloServer()
{
          this->m_clientVec.clear();
          ::closesocket(this->m_server_socket);
#ifdef _WINDOWS
          WSACleanup();
#endif // _WINDOWS
}

/*------------------------------------------------------------------------------------------------------
* use socket api to create a ipv4 and tcp protocol socket 
* @function��static SOCKET createServerSocket
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
* @function��static SOCKET createServerSocket
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
* @function��static bool acceptClientConnection
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
          int addrlen = sizeof(SOCKADDR_IN);
          *clientSocket =  ::accept(
                    serverSocket, 
                    reinterpret_cast<sockaddr*>(_clientAddr),
                    &addrlen
          );
          if (*clientSocket == INVALID_SOCKET) {                 //socket error occured!
                    return false;
          }

          std::cout << "[CLIENT MEMBER JOINED]: IP Address = " 
                    << inet_ntoa(_clientAddr->sin_addr)
                    << ", Port = " << _clientAddr->sin_port << std::endl;
          return true;
}

/*------------------------------------------------------------------------------------------------------
* init server ip address and port and bind it with server socket
* @function��void initServerAddressBinding
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
#ifdef _WINDOWS
          this->m_server_address.sin_addr.S_un.S_addr = _ipAddr;                 //IP address(Windows)   
#else
          this->m_server_address.sin_addr.s_addr = _ipAddr;                            //IP address(LINUX)   
#endif // _WINDOWS

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
* @function��void initServerAddressBinding
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
* @function��void sendDataToClient
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
* @function��void reciveDataFromClient
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

void HelloServer::serverMainFunction()
{
          while (1) {
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

                    if (::select(static_cast<int>(this->m_server_socket + 1),
                              &m_fdread,
                              &m_fdwrite,
                              &m_fdexception,
                              reinterpret_cast<const timeval*>(&this->m_timeoutSetting)) < 0) {     //Select Task Ended!
                              break;
                    }
                    /*SERVER ACCELERATION PROPOSESD*/
                    if (this->m_fdread.fd_count) {                                         //in fd_read array, no socket has been found!!               

                    }

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
                    }

                    /*Currently,there is no any client is the container, so there is no reason to excute following instructions */
                    if (!this->m_clientVec.size()) {
                              for (std::vector<_ClientAddr>::iterator ib = this->m_clientVec.begin(); ib != this->m_clientVec.end();) {
                                     /*
                                     *Entering main logic layer std::vector<_ClientAddr>::iterator as an input to the main system
                                     * retvalue: when functionlogicLayer return a value of false it means [CLIENT EXIT MANUALLY]]
                                     */
                                        if (!this->funtionLogicLayer(ib)) {

                                                  /*There is a kind of sceniro which there is only one client who still remainning connection to the server*/
                                                  if (ib == this->m_clientVec.end()) {		             //Erase Current unavailable client's socket
                                                            this->m_clientVec.erase(ib);
                                                            break;
                                                  }
                                                  else {
                                                            ib = this->m_clientVec.erase(ib);
                                                  }
                                                  ib++;
                                        }
                              }
                    }
          }
}

/*------------------------------------------------------------------------------------------------------
* @function��void funtionLogicLayer
* @param: [IN] std::vector<_ClientAddr>::iterator
* @description: process the request from clients
* @retvalue : bool
*------------------------------------------------------------------------------------------------------*/
bool HelloServer::funtionLogicLayer(IN std::vector<_ClientAddr>::iterator  _clientSocket)
{
          _PackageHeader packageHeader;
          int  recvStatus = this->reciveDataFromClient(      // record recv data status
                    _clientSocket->m_clientSocket,
                    &packageHeader, 
                    sizeof(_PackageHeader)
          ); 
          if (recvStatus <= 0) {                                              //Client Exit Manually 
                    return false;
          }
         
          std::cout << "[CLIENT COMMAND MESSAGE] FROM IP Address = "
                    << inet_ntoa(_clientSocket->m_clientAddr.sin_addr)
                    << ",Port=" << _clientSocket->m_clientAddr.sin_port << std::endl
                    << "->Command= " << packageHeader._packageCmd << std::endl
                    << "->PackageLength= " << packageHeader._packageLength << std::endl;

          if (packageHeader._packageCmd == CMD_LOGIN) {
                    _LoginData loginData;
                    recvStatus = this->reciveDataFromClient(
                              _clientSocket->m_clientSocket,
                              reinterpret_cast<char*>(&loginData) + sizeof(_PackageHeader),
                              packageHeader._packageLength - sizeof(_PackageHeader)
                    );
                    loginData.loginStatus = true;                                                                               //set login status as true

                    std::cout << "[CLIENT LOGIN MESSAGE] FROM IP Address = "
                              << inet_ntoa(_clientSocket->m_clientAddr.sin_addr)
                              << ",Port=" << _clientSocket->m_clientAddr.sin_port << std::endl
                              << "->UserName= " << loginData.userName << std::endl
                              << "->UserPassword= " << loginData.userPassword << std::endl;

                    this->sendDataToClient(_clientSocket->m_clientSocket, &loginData, sizeof(loginData));
          }
          else if (packageHeader._packageCmd == CMD_LOGOUT) {
                    _LogoutData logoutData;
                    recvStatus = this->reciveDataFromClient(
                              _clientSocket->m_clientSocket,
                              reinterpret_cast<char*>(&logoutData) + sizeof(_PackageHeader),
                              packageHeader._packageLength - sizeof(_PackageHeader)
                    );
                    logoutData.logoutStatus = true;                                                                               //set logout status as true
                    std::cout << "[CLIENT LOGOUT MESSAGE] FROM IP Address = "
                              << inet_ntoa(_clientSocket->m_clientAddr.sin_addr)
                              << ",Port=" << _clientSocket->m_clientAddr.sin_port << std::endl
                              << "->UserName= " << logoutData.userName << std::endl;

                    this->sendDataToClient(_clientSocket->m_clientSocket, &logoutData, sizeof(logoutData));
          }
          else if (packageHeader._packageCmd == CMD_SYSTEM) {
                    _SystemData systemData("Server System", "100");

                    std::cout << "[CLIENT SYSTEM MESSAGE] FROM IP Address = "
                              << inet_ntoa(_clientSocket->m_clientAddr.sin_addr)
                              << ",Port=" << _clientSocket->m_clientAddr.sin_port << std::endl;

                    this->sendDataToClient(_clientSocket->m_clientSocket, &systemData, sizeof(_SystemData));
          }
          if (recvStatus <= 0) {                                              //Client Exit Manually
                    return false;
          }  
          return true;
}
