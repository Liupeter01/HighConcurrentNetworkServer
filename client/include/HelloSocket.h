#pragma once
#define _WINDOWS
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif
#include<iostream>
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
public:
          void sendDataToServer(IN const char* _szSendBuf);
          void reciveDataFromServer(
                    OUT char* _szRecvBuf, 
                    OUT int &_szBufferSize
          );

private:
#ifdef _WINDOWS
          WSADATA m_wsadata;
#endif // _WINDOWS 
          SOCKET m_client_socket;                           //client connection socket
          sockaddr_in m_client__address;
};