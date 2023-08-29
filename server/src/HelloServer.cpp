#include"HelloServer.h"

HelloServer::HelloServer()
          : m_client_connect_address{0},
          m_server_address{0},
          m_client_connect_socket(INVALID_SOCKET),
          m_server_socket(INVALID_SOCKET)
{
}

HelloServer::HelloServer(unsigned short _ipPort)
          :HelloServer(INADDR_ANY,_ipPort)
{

}

/*------------------------------------------------------------------------------------------------------
* @function£ºHelloServer::HelloServer
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
#ifdef _WINDOWS
          WSACleanup();
#endif // _WINDOWS
}

/*------------------------------------------------------------------------------------------------------
* use socket api to create a ipv4 and tcp protocol socket 
* @function£ºstatic SOCKET createServerSocket
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
* @function£ºstatic SOCKET createServerSocket
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
* @function£ºstatic SOCKET acceptClientConnection
* @param :
*                   1.[IN] SOCKET  serverSocket
*                   2.[IN OUT] SOCKADDR_IN* _clientAddr
* 
* @retvalue: SOCKET
*------------------------------------------------------------------------------------------------------*/
SOCKET HelloServer::acceptClientConnection(
          IN SOCKET serverSocket,
          IN OUT SOCKADDR_IN* _clientAddr)
{
          int addrlen = sizeof(SOCKADDR_IN);
          return ::accept(
                    serverSocket, 
                    reinterpret_cast<sockaddr*>(_clientAddr),
                    &addrlen
          );
}

/*------------------------------------------------------------------------------------------------------
* init server ip address and port and bind it with server socket
* @function£ºvoid initServerAddressBinding
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
          
          if (this->m_client_connect_socket == INVALID_SOCKET) {
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
* @function£ºvoid initServerAddressBinding
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
* server accept client's connection
* @function£ºserverAcceptConnetion
*------------------------------------------------------------------------------------------------------*/
void HelloServer::serverAcceptConnetion()
{
          this->m_client_connect_socket = this->acceptClientConnection(                     //
                    this->m_server_socket,
                    &this->m_client_connect_address
          );
          if (this->m_client_connect_socket == INVALID_SOCKET) {                 //socket error occured!
                    return;
          }
          std::cout << "[CLIENT MEMBER JOINED]: IP Address = " << inet_ntoa(this->m_client_connect_address.sin_addr) << std::endl;
}

/*------------------------------------------------------------------------------------------------------
* @function£ºvoid sendDataToClient
* @param : [IN] const char *_szBuf
*------------------------------------------------------------------------------------------------------*/
void HelloServer::sendDataToClient(IN const char* _szSendBuf)
{
          ::send(
                    this->m_client_connect_socket,
                    _szSendBuf,
                    static_cast<int>(strlen(_szSendBuf)+1),
                    0
          );
}

/*------------------------------------------------------------------------------------------------------
* @function£ºvoid reciveDataFromClient
* @param :
                    1. [OUT] char* _szRecvBuf
                    2. [IN] int &_szBufferSize
* @retvalue: int  
*------------------------------------------------------------------------------------------------------*/
int HelloServer::reciveDataFromClient(
          OUT char* _szRecvBuf,
          IN int _szBufferSize)
{
          return  ::recv(
                    this->m_client_connect_socket,
                    _szRecvBuf,
                    _szBufferSize,
                    0
          );
}

void HelloServer::serverMainFunction()
{
          this->serverAcceptConnetion();
          while (1) {
                    char str[256]{ 0 };
                    int recvStatus = this->reciveDataFromClient(str, sizeof(str) / sizeof(char));
                    std::cout << "[CLIENT MESSAGE] FROM IP Address = "
                              << inet_ntoa(this->m_client_connect_address.sin_addr)
                              << "  Message Info: ";

                    /*Client Exited Server*/
                    if (recvStatus <= 0) {
                              std::cout << "Client Exited" << std::endl;
                              break;
                    }
                    else {
                              std::cout << str << std::endl;
                    }

                    /*Client Still Connect to Server*/
                    if (!strcmp(str, "getName")) {
                              this->sendDataToClient("Valid Server Name Request");
                    }
                    else if (!strcmp(str,"getAge")) {
                              this->sendDataToClient("Valid Server Run Time Request");
                    }
                    else {
                              this->sendDataToClient("Invalid Request");
                    }
          }
}