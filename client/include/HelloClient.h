#pragma once
#define _WINDOWS
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#include "DataPackage.h"

#include<iostream>
#include<cassert>

class HelloClient
{
public:
          HelloClient();
          virtual ~HelloClient();

public:
          static SOCKET createClientSocket(
                    IN int af = AF_INET,
                    IN int type = SOCK_STREAM,
                    IN int protocol = IPPROTO_TCP
          );

public:
          void connectServer(
                    IN unsigned long _ipAddr, 
                    IN unsigned short _ipPort
          );

          void sendDataToServer(
                    IN const char* _szSendBuf,
                    IN int _szBufferSize
          );

          void reciveDataFromServer(
                    OUT char* _szRecvBuf, 
                    IN OUT int _szBufferSize
          );
          void clientMainFunction();

private:
#ifdef _WINDOWS
          WSADATA m_wsadata;
#endif // _WINDOWS 
          SOCKET m_client_socket;                           //client connection socket
          sockaddr_in m_server_address;
};