#include"HelloSocket.h"

HelloSocket::HelloSocket()
{
#ifdef _WINDOWS
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif // _WINDOWS
}

HelloSocket::~HelloSocket()
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
SOCKET HelloSocket::createServerSocket(
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
*                   1.[IN] SOCKET  socket
*                   2.[IN] int backlog
*------------------------------------------------------------------------------------------------------*/
int HelloSocket::startListeningConnection(
          IN SOCKET  socket,
          IN int backlog)
{
          return ::listen(socket, backlog);
}

/*------------------------------------------------------------------------------------------------------
* init server ip address and port and bind it with server socket
* @function£ºvoid initServerAddressBinding
* @param : unsigned short _port
*------------------------------------------------------------------------------------------------------*/
void HelloSocket::initServerAddressBinding(unsigned short _port)
{
          int bindStatus = -1;                                                                                   //::bind() function return value
          this->m_server_socket = createServerSocket();
          
          assert(this->m_server_socket != INVALID_SOCKET);                        //socket error occured!

          this->m_server_address.sin_family = AF_INET;                                    //IPV4
          this->m_server_address.sin_addr.S_un.S_addr = INADDR_ANY;        //IP address
          this->m_server_address.sin_port = htons(_port);                                     //Port number

          bindStatus = ::bind(
                    this->m_server_socket,
                    (sockaddr*)&this->m_server_address,
                    sizeof(this->m_server_address)
          );

          assert(bindStatus != SOCKET_ERROR);                                               //error occured!
}

/*------------------------------------------------------------------------------------------------------
* init server ip address and port and bind it with server socket
* @function£ºvoid initServerAddressBinding
* @param : unsigned short _port
*------------------------------------------------------------------------------------------------------*/
void  HelloSocket::startServerListening(
          unsigned short _listenport,
          int _listenNumber)
{
          int listenStatus = -1;

          this->initServerAddressBinding(_listenport);                                    //create server socket and bind ip address info
          listenStatus =  startListeningConnection(this->m_server_socket);   //start listening on current server ip:port
         
          assert(listenStatus != SOCKET_ERROR);                                          //listen error occured!
}