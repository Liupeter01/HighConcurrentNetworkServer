#pragma once
#define _WINDOWS
#ifdef _WINDOWS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif
#include<iostream>
#include<cassert>

class HelloServer
{
public:
          HelloServer();
          HelloServer(IN unsigned short _ipPort);
          HelloServer(IN unsigned long _ipAddr, IN unsigned short _ipPort);
          virtual ~HelloServer();

public:

          static SOCKET createServerSocket(
                    IN int af = AF_INET,
                    IN int type = SOCK_STREAM,
                    IN int protocol = IPPROTO_TCP
          );
          static int startListeningConnection(
                    IN SOCKET serverSocket,
                    IN int backlog = SOMAXCONN
          );
          static SOCKET acceptClientConnection(
                    IN SOCKET serverSocket,
                    IN OUT SOCKADDR_IN *_clientAddr
          );

public:
          void startServerListening(int backlog = SOMAXCONN);
          void serverMainFunction();
          void sendDataToClient(IN const char* _szSendBuf);
          void reciveDataFromClient(
                    OUT char* _szRecvBuf,
                    OUT int _szBufferSize
          );

private:
          void serverAcceptConnetion();
          void initServerAddressBinding(
                    unsigned long _ipAddr,
                    unsigned short _port
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