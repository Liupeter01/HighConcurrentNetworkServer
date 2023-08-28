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
* 
* @function£ºvoid sendDataToServer
* @param : [IN] const char *_szBuf
*------------------------------------------------------------------------------------------------------*/
void HelloSocket::sendDataToServer(IN const char *_szSendBuf)
{
          ::send(
                    this->m_client_socket,
                    _szSendBuf,
                    static_cast<int>(strlen(_szSendBuf) + 1),
                    0
          );
}

/*------------------------------------------------------------------------------------------------------
* 
* @function£ºvoid reciveDataFromServer
* @param : 
                    1. [OUT] char* _szRecvBuf 
                    2. [OUT] int &_szBufferSize
*------------------------------------------------------------------------------------------------------*/
void HelloSocket::reciveDataFromServer(
          OUT char* _szRecvBuf,
          OUT int &_szBufferSize)
{
          ::recv(
                    this->m_client_socket, 
                    _szRecvBuf, 
                    _szBufferSize, 
                    0
          );
}