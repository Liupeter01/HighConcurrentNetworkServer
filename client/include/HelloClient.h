#pragma once
#define _WINDOWS
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#include<DataPackage.h>
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

          template<typename T> void sendDataToServer(
                    IN  SOCKET& _clientSocket,
                    IN T* _szSendBuf,
                    IN int _szBufferSize);

          template<typename T> int reciveDataFromServer(
                    IN  SOCKET& _clientSocket,
                    OUT T* _szRecvBuf,
                    IN int _szBufferSize
          );

          void clientMainFunction();

private:
          void initClientIOMultiplexing();
          bool initClientSelectModel();
          bool functionLogicLayer();

private:
          fd_set m_fdread;
          timeval m_timeoutSetting{ 0/*0 s*/, 0 /*0 ms*/ };

          SOCKET m_client_socket;                           //client connection socket
          sockaddr_in m_server_address;
#ifdef _WINDOWS
          WSADATA m_wsadata;
#endif // _WINDOWS 
};