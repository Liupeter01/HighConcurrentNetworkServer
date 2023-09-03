#pragma once
#ifndef _HELLOCLIENT_H_
#define _HELLOCLIENT_H_
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
typedef uint64_t UINT_PTR, * PUINT_PTR;
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

          virtual void clientInterfaceLayer(
                    IN SOCKET& _client,
                    IN OUT  std::promise<bool>& interfacePromise
          );

          bool dataProcessingLayer();

          template<typename T>
          void readMessageHeader(IN T* _header);

          virtual void readMessageBody(IN _PackageHeader* _buffer);

private:
          /*client interface thread*/
          std::promise<bool> m_interfacePromise;
          std::shared_future<bool> m_interfaceFuture;

          /*select network model*/
          fd_set m_fdread;
          timeval m_timeoutSetting{ 0/*0 s*/, 0 /*0 ms*/ };

          /*client socket and server address*/
          SOCKET m_client_socket;                           //client connection socket
          sockaddr_in m_server_address;

          /*memory buffer*/
          const unsigned int m_szRecvBufSize = 4096 * 1024;                  //4MB
          std::shared_ptr<char> m_szRecvBuffer;                                       //server recive buffer(retrieve much data as possible from kernel)

          const unsigned int m_szMsgBufSize = 4096 * 1024 * 10;           //40MB
          unsigned long m_szMsgPtrPos = 0;                                               //message pointer location pos
          std::shared_ptr<char> m_szMsgBuffer;                                        //find available data from server recive buffer

#if _WIN32 
          WSADATA m_wsadata;
#endif // _WINDOWS 
};

#endif 