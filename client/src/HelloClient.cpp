#include"HelloClient.h"

HelloClient::HelloClient()
{
#ifdef _WINDOWS
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif // _WINDOWS
          this->m_client_socket = this->createClientSocket();
          if (this->m_client_socket == INVALID_SOCKET) {
                    return;
          }
}

HelloClient::~HelloClient()
{
#ifdef _WINDOWS
          WSACleanup();
#endif // _WINDOWS
}

/*------------------------------------------------------------------------------------------------------
* use socket api to create a ipv4 and tcp protocol socket 
* @function£ºstatic SOCKET createClientSocket
* @param :
*                   1.[IN] int af
*                   2.[IN] int type
*                   3.[IN] int protocol
*------------------------------------------------------------------------------------------------------*/
SOCKET HelloClient::createClientSocket(
          IN int af,
          IN int type,
          IN int protocol)
{
          return  ::socket(af, type, protocol);                              //Create server socket
}

/*------------------------------------------------------------------------------------------------------
* user should input server's ip:port info to establish the connection
* @function£ºvoid connectServer
* @param : 
*                   1.[IN] unsigned long _ipAddr
*                   2.[IN] unsigned short _ipPort
*------------------------------------------------------------------------------------------------------*/
void HelloClient::connectServer(
          IN unsigned long _ipAddr,
          IN unsigned short _ipPort
)
{
          this->m_server_address.sin_family = AF_INET;                                    //IPV4
          this->m_server_address.sin_port = htons(_ipPort);                                     //Port number
#ifdef _WINDOWS
          this->m_server_address.sin_addr.S_un.S_addr = _ipAddr;                 //IP address(Windows)   
#else
          this->m_server_address.sin_addr.s_addr = _ipAddr;                            //IP address(LINUX)   
#endif // _WINDOWS

          if (::connect(this->m_client_socket,
                    reinterpret_cast<sockaddr*>(&this->m_server_address),
                    sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
                    return;
          }
}

/*------------------------------------------------------------------------------------------------------
* @function£ºvoid sendDataToServer
* @param :
*                  1.[IN] SOCKET& _clientSocket,
                    2.[IN] T*_szSendBuf
                    3.[IN] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
template<typename T>
void HelloClient::sendDataToServer(
          IN  SOCKET& _clientSocket,
          IN T* _szSendBuf,
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
* @function£ºvoid reciveDataFromServer
* @param :
*                  1. IN  SOCKET& _clientSocket
                    2. [OUT] T* _szRecvBuf
                    3. [IN OUT] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
template<typename T> 
int HelloClient::reciveDataFromServer(
          IN  SOCKET& _clientSocket,
          OUT T* _szRecvBuf,
          IN int _szBufferSize)
{
          return ::recv(
                    _clientSocket,
                    reinterpret_cast<char*>(_szRecvBuf),
                    _szBufferSize,
                    0
          );
}

/*------------------------------------------------------------------------------------------------------
* @function£ºvoid clientMainCFunction
*------------------------------------------------------------------------------------------------------*/
void HelloClient::clientMainFunction()
{
          while (true) 
          {
                    this->initClientIOMultiplexing();
                    if (this->initClientSelectModel()) {
                              break;
                    }
                    /*CLIENT ACCELERATION PROPOSESD*/
                    if (!this->m_fdread.fd_count) {                                         //in fd_read array, no socket has been found!!               

                    }
                    if (FD_ISSET(this->m_client_socket, &this->m_fdread)) {
                              FD_CLR(this->m_client_socket, &this->m_fdread);
                              if (!this->functionLogicLayer()) {                          //Client Exit Manually
                                        break;
                              }
                    }
          }
}

/*------------------------------------------------------------------------------------------------------
* init IO Multiplexing
* @function: void initClientIOMultiplexing
* @description: in client, we only need to deal with client socket 
*------------------------------------------------------------------------------------------------------*/
void HelloClient::initClientIOMultiplexing()
{
          FD_ZERO(&m_fdread);                                                              //clean fd_read
          FD_SET(this->m_client_socket, &m_fdread);                           //Insert Server Socket into fd_read
}

/*------------------------------------------------------------------------------------------------------
* @function£ºbool initClientSelectModel
*------------------------------------------------------------------------------------------------------*/
bool HelloClient::initClientSelectModel()
{
          return (::select(static_cast<int>(this->m_client_socket + 1),
                    &m_fdread,
                    nullptr,
                    nullptr,
                    reinterpret_cast<const timeval*>(&this->m_timeoutSetting)) < 0);                  //Select Task Ended!
}

/*------------------------------------------------------------------------------------------------------
* @function£ºbool functionLogicLayer
*------------------------------------------------------------------------------------------------------*/
bool HelloClient::functionLogicLayer()
{
          char _Message[256]{ 0 };
          std::cin.getline(_Message, 256);
          if (!strcmp(_Message, "exit")) {
                    std::cout << "[CLIENT EXIT] Client Exit Manually" << std::endl;
                    return false;
          }
          else if (!strcmp(_Message, "login")) {
                    _LoginData loginData("client-loopback404", "1234567abc");
                    _LoginData recvLoginData;

                    this->sendDataToServer(this->m_client_socket, &loginData, sizeof(loginData));

                    this->reciveDataFromServer(this->m_client_socket, &recvLoginData, sizeof(_LoginData));
                    if (recvLoginData.loginStatus) {
                              std::cout << "[CLIENT LOGIN INFO] Message Info: " << std::endl
                                        << "->userName = " << recvLoginData.userName << std::endl
                                        << "->userPassword = " << recvLoginData.userPassword << std::endl;
                    }
          }
          else if (!strcmp(_Message, "logout")) {
                    _LogoutData logoutData("client-loopback404");
                    _LogoutData recvLogoutData;
                    this->sendDataToServer(this->m_client_socket, &logoutData, sizeof(logoutData));

                    this->reciveDataFromServer(this->m_client_socket, &recvLogoutData, sizeof(_LogoutData));
                    if (recvLogoutData.logoutStatus) {
                              std::cout << "[CLIENT LOGOUT INFO] Message Info: " << std::endl
                                        << "->userName = " << recvLogoutData.userName << std::endl;
                    }
          }
          else if (!strcmp(_Message, "system")) {
                    _SystemData systemData;
                    this->sendDataToServer(this->m_client_socket, &systemData, sizeof(_PackageHeader));
                    this->reciveDataFromServer(this->m_client_socket, &systemData, sizeof(_SystemData));
                    std::cout << "[SERVER INFO] Message Info: " << std::endl
                              << "->serverName = " << systemData.serverName << std::endl
                              << "->serverRunTime = " << systemData.serverRunTime << std::endl;
          }
          else {
                    std::cout << "[CLIENT ERROR INFO] Invalid Command Input!" << std::endl;
          }
          return true;
}
