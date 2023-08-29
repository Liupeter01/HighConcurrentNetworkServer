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

          int connectStatus = ::connect(
                    this->m_client_socket,
                    reinterpret_cast<sockaddr*>(&this->m_server_address),
                    sizeof(SOCKADDR_IN)
          );
          if (connectStatus == SOCKET_ERROR) {
                    return;
          }
}

/*------------------------------------------------------------------------------------------------------
* @function£ºvoid sendDataToServer
* @param :
                    1.[IN] const char *_szBuf
                    2.[IN] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
void HelloClient::sendDataToServer(
          IN const char* _szSendBuf,
          IN int _szBufferSize)
{
          ::send(
                    this->m_client_socket,
                    _szSendBuf,
                    _szBufferSize,
                    0
          );
}

/*------------------------------------------------------------------------------------------------------
* @function£ºvoid reciveDataFromServer
* @param : 
                    1. [OUT] char* _szRecvBuf 
                    2. [IN OUT] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
void HelloClient::reciveDataFromServer(
          OUT char* _szRecvBuf,
          IN OUT int _szBufferSize)
{
          ::recv(
                    this->m_client_socket, 
                    _szRecvBuf, 
                    _szBufferSize,
                    0
          );
}

/*------------------------------------------------------------------------------------------------------
* @function£ºvoid clientMainCFunction
*------------------------------------------------------------------------------------------------------*/
void HelloClient::clientMainFunction()
{
          while (true) {
                    char _Message[256]{ 0 };
                    std::cin.getline(_Message, 256);
                    if (!strcmp(_Message, "exit")) {
                              std::cout << "[CLIENT EXIT] Client Exit Manually" << std::endl;
                              break;
                    }
                    else if (strlen(_Message) != 0) {
                              this->sendDataToServer(
                                        _Message, 
                                        sizeof(_Message) / sizeof(char)
                              );
                    }

                    memset(_Message, 0, sizeof(_Message));
                    this->reciveDataFromServer(_Message, sizeof(_Message) / sizeof(char));
                    DataPackage* dataPackage = reinterpret_cast<DataPackage*>(_Message);
                    std::cout << "[SERVER INFO] Message Info: " << std::endl
                              << "->serverName = " << dataPackage->serverName << std::endl
                              << "->serverRunTime = " << dataPackage->serverRunTime << std::endl;
          }
}