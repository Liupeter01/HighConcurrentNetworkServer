#pragma once
#include<DataPackage.h>
#include<iostream>
#include<cassert>
#include<future>
#include<thread>

#if _WIN32                          //Windows Enviorment
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

#else                                   //Unix/Linux/Macos Enviorment

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

          void functionClientInput(IN SOCKET& _client);
          bool functionLogicLayer();

private:
          /*client interface thread*/
          std::promise<bool> m_interfacePromise;
          std::shared_future<bool> m_interfaceFuture = this->m_interfacePromise.get_future();

          /*select network model*/
          fd_set m_fdread;
          timeval m_timeoutSetting{ 0/*0 s*/, 0 /*0 ms*/ };

          /*client socket and server address*/
          SOCKET m_client_socket;                           //client connection socket
          sockaddr_in m_server_address;

#if _WIN32 
          WSADATA m_wsadata;
#endif // _WINDOWS 
};