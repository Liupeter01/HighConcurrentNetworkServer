#pragma once
#define _WINDOWS
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#include<cassert>

class HelloSocket
{
public:
          HelloSocket();
          virtual ~HelloSocket();

public:
          static SOCKET createServerSocket(
                    IN int af = AF_INET,
                    IN int type = SOCK_STREAM,
                    IN int protocol = IPPROTO_TCP
          );
          static int startListeningConnection(
                    IN SOCKET socket,
                    IN int backlog = SOMAXCONN
          );

private:
          void initServerAddressBinding(unsigned short _port);
          void startServerListening(
                    unsigned short _listenport,
                    int _listenNumber = SOMAXCONN
          );

private:
#ifdef _WINDOWS
          WSADATA m_wsadata;
#endif // _WINDOWS 
          SOCKET m_server_socket;                           //server listening socket
          sockaddr_in m_server_address;

          SOCKET m_client_connect_socket;             //client connection socket
          sockaddr_in m_client_connect_address;
};