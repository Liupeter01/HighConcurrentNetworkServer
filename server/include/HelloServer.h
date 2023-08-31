#pragma once
#include "DataPackage.h"
#include<iostream>
#include<cassert>
#include<vector>
#include<future>
#include<thread>

#if _WIN32 || WIN3 /* Windows Enviorment*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

#else              /* Unix/Linux/Macos Enviorment*/

#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>

/* Network Socket Def*/
typedef uint64_t UINT_PTR,*PUINT_PTR;
typedef UINT_PTR SOCKET;
typedef struct timeval timeval;
typedef sockaddr_in SOCKADDR_IN;
#define INVALID_SOCKET static_cast<SOCKET>(~0)
#define SOCKET_ERROR (-1)

/*param sign*/
#define IN          //param input sign
#define OUT         //param output sign
#endif

struct _ClientAddr{
          _ClientAddr();
          _ClientAddr(SOCKET _socket, sockaddr_in _addr);
          virtual ~_ClientAddr();
          SOCKET m_clientSocket;
          sockaddr_in m_clientAddr;
};

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
          static bool acceptClientConnection(
                    IN SOCKET serverSocket,
                    OUT SOCKET* clientSocket,
                    OUT SOCKADDR_IN* _clientAddr
          );

public:
          template<typename T> void sendDataToClient(
                    IN  SOCKET& _clientSocket,
                    IN T* _szSendBuf, 
                    IN int _szBufferSize
          );

          template<typename T> int reciveDataFromClient(
                    IN  SOCKET& _clientSocket,
                    OUT T* _szRecvBuf,
                    IN int _szBufferSize
          );
          void startServerListening(int backlog = SOMAXCONN);
          void serverMainFunction();

private:
          void initServerAddressBinding(
                    unsigned long _ipAddr,
                    unsigned short _port
          );

          void initServerIOMultiplexing();
          bool initServerSelectModel();
          bool functionLogicLayer(IN std::vector<_ClientAddr>::iterator _clientSocket);
          bool functionServerLayer();

private:
          fd_set m_fdread;
          fd_set m_fdwrite;
          fd_set m_fdexception;
          timeval m_timeoutSetting{ 0/*0 s*/, 0 /*0 ms*/ };

          SOCKET m_server_socket;                           //server listening socket
          sockaddr_in m_server_address;
          std::vector<_ClientAddr> m_clientVec;

#ifdef _WINDOWS
          WSADATA m_wsadata;
#endif // _WINDOWS 
};