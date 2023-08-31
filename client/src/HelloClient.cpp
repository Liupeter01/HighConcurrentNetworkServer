#include"HelloClient.h"

HelloClient::HelloClient()
{
#if _WIN32                          //Windows Enviormen
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif
          this->m_client_socket = this->createClientSocket();
          if (this->m_client_socket == INVALID_SOCKET) {
                    return;
          }
}

HelloClient::~HelloClient()
{
#if _WIN32                                                   //Windows Enviorment
          ::shutdown(this->m_client_socket, SD_BOTH); //disconnect I/O
          ::closesocket(this->m_client_socket);     //release socket completely!! 
          ::WSACleanup();

#else                                                                  //Unix/Linux/Macos Enviorment
          ::shutdown(this->m_client_socket, SHUT_RDWR);//disconnect I/O and keep recv buffer
          close(this->m_client_socket);                //release socket completely!! 

#endif
}

/*------------------------------------------------------------------------------------------------------
* use socket api to create a ipv4 and tcp protocol socket 
* @function:static SOCKET createClientSocket
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
* @function:void connectServer
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

#if _WIN32    //Windows Enviorment
          this->m_server_address.sin_addr.S_un.S_addr = _ipAddr;                 //IP address(Windows)   
#else               /* Unix/Linux/Macos Enviorment*/
          this->m_server_address.sin_addr.s_addr = _ipAddr;                            //IP address(LINUX) 
#endif

          if (::connect(this->m_client_socket,
                    reinterpret_cast<sockaddr*>(&this->m_server_address),
                    sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
                    return;
          }
}

/*------------------------------------------------------------------------------------------------------
* @function:void sendDataToServer
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
* @function:void reciveDataFromServer
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
*  Currently, clientMainFunction excute on a new thread(std::thread m_clientInterface)
* @function: void functionClientInput
* @param: 
                    1.[IN] SOCKET & _client
                    2.[IN OUT]  std::promise<bool> &interfacePromise
*------------------------------------------------------------------------------------------------------*/
void HelloClient::functionClientInput(
          IN SOCKET& _client,
          IN OUT  std::promise<bool> &interfacePromise)
{
          while (true) {
                    char _Message[256]{ 0 };
                    std::cin.getline(_Message, 256);
                    if (!strcmp(_Message, "exit")) {
                              std::cout << "[CLIENT EXIT] Client Exit Manually" << std::endl;
                              interfacePromise.set_value(false);                            //set symphore value to inform other thread
                              return;
                    }
                    else if (!strcmp(_Message, "login")) {
                              _LoginData loginData("client-loopback404", "1234567abc");
                              this->sendDataToServer(_client, &loginData, sizeof(loginData));
                    }
                    else if (!strcmp(_Message, "logout")) {
                              _LogoutData logoutData("client-loopback404");
                              this->sendDataToServer(_client, &logoutData, sizeof(logoutData));
                    }
                    else if (!strcmp(_Message, "system")) {
                              _SystemData systemData;
                              this->sendDataToServer(_client, &systemData, sizeof(_PackageHeader));
                    }
                    else {
                              std::cout << "[CLIENT ERROR INFO] Invalid Command Input!" << std::endl;
                    }
          }
}

/*------------------------------------------------------------------------------------------------------
* @function:bool functionLogicLayer
*------------------------------------------------------------------------------------------------------*/
bool HelloClient::functionLogicLayer()
{
          char _buffer[256]{ 0 };
          if (this->reciveDataFromServer(this->m_client_socket, &_buffer, sizeof(_PackageHeader))<=0) {
                    return false;
          }
          if ((reinterpret_cast<_PackageHeader*>(_buffer))->_packageCmd == CMD_LOGIN) {
                    _LoginData* recvLoginData(reinterpret_cast<_LoginData*>(_buffer));
                    this->reciveDataFromServer(
                              this->m_client_socket,
                              _buffer + sizeof(_PackageHeader),
                              (reinterpret_cast<_PackageHeader*>(_buffer))->_packageLength - sizeof(_PackageHeader)
                    );
                    if (recvLoginData->loginStatus) {
                              std::cout << "[CLIENT LOGIN INFO] Message Info: " << std::endl
                                        << "->userName = " << recvLoginData->userName << std::endl
                                        << "->userPassword = " << recvLoginData->userPassword << std::endl;
                    }
          }
          else if ((reinterpret_cast<_PackageHeader*>(_buffer))->_packageCmd == CMD_LOGOUT) {
                    _LogoutData* recvLogoutData(reinterpret_cast<_LogoutData*>(_buffer));
                    this->reciveDataFromServer(
                              this->m_client_socket,
                              _buffer + sizeof(_PackageHeader),
                              (reinterpret_cast<_PackageHeader*>(_buffer))->_packageLength - sizeof(_PackageHeader)
                    );
                    if (recvLogoutData->logoutStatus) {
                              std::cout << "[CLIENT LOGOUT INFO] Message Info: " << std::endl
                                        << "->userName = " << recvLogoutData->userName << std::endl;
                    }
          }
          else if ((reinterpret_cast<_PackageHeader*>(_buffer))->_packageCmd == CMD_SYSTEM) {
                    _SystemData* systemData(reinterpret_cast<_SystemData*>(_buffer));
                    this->reciveDataFromServer(
                              this->m_client_socket,
                              _buffer + sizeof(_PackageHeader),
                              (reinterpret_cast<_PackageHeader*>(_buffer))->_packageLength - sizeof(_PackageHeader)
                    );
                    std::cout << "[SERVER INFO] Message Info: " << std::endl
                              << "->serverName = " << systemData->serverName << std::endl
                              << "->serverRunTime = " << systemData->serverRunTime << std::endl;
          }
          else {
                    std::cout << "[CLIENT UNKOWN INFO] Message Info: Unkown Command" << std::endl;
          }
          return true;
}

/*------------------------------------------------------------------------------------------------------
* Currently, clientMainFunction only excute on the main Thread
* @function:void clientMainFunction
*------------------------------------------------------------------------------------------------------*/
void HelloClient::clientMainFunction()
{
          auto res = std::async(                                                                              //shared_future requires std::async to startup
                    std::launch::async,
                    &HelloClient::functionClientInput, this,
                    std::ref(this->m_client_socket),
                    std::ref(this->m_interfacePromise)
          );
          while (this->m_interfaceFuture.get())
          {
                    this->initClientIOMultiplexing();
                    if (this->initClientSelectModel()) {
                              break;
                    }

#if _WIN32     
                    /*CLIENT ACCELERATION PROPOSESD (Windows Enviorment only!)*/
                    if (!this->m_fdread.fd_count) {                                         //in fd_read array, no socket has been found!!               
                    }
#endif

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
* @function: bool initClientSelectModel
*------------------------------------------------------------------------------------------------------*/
bool HelloClient::initClientSelectModel()
{
          return (::select(static_cast<int>(this->m_client_socket + 1),
                    &m_fdread,
                    nullptr,
                    nullptr,
                    &this->m_timeoutSetting) < 0);                  //Select Task Ended!
}