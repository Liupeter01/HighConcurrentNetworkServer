#pragma once
#ifndef _SERVERSOCKET_H_
#define _SERVERSOCKET_H_
#include<DataPackage.h>
#include<iostream>
#include<cassert>
#include<future>
#include<thread>
#include<atomic>

#if _WIN32                          //Windows Enviorment
/*break the limitaion of the select model size*/
#define FD_SETSIZE 1024      
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable : 4005)

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

struct _ServerSocket
{
public:
          _ServerSocket();
          _ServerSocket(
                    IN SOCKET& _socket,
                    IN sockaddr_in& _addr
          );

          virtual ~_ServerSocket();

public:
          /*Get Server Socket and Address Info*/
          SOCKET& getServerSocket();
          const in_addr& getServerAddr()const;
          const unsigned short& getServerPort()const;

          /*Operate Current Client's Send and Recv Buffer*/
          unsigned int getMsgPtrPos() const;
          unsigned int getBufFullSpace() const;
          unsigned int getBufRemainSpace() const;

          char* getMsgBufferHead();
          char* getMsgBufferTail();

          void increaseMsgBufferPos(unsigned int _increaseSize);
          void decreaseMsgBufferPos(unsigned int _decreaseSize);
          void resetMsgBufferPos();

          unsigned int getSendPtrPos() const;
          unsigned int getSendBufFullSpace() const;
          unsigned int getSendBufRemainSpace() const;

          char* getSendBufferHead();
          char* getSendBufferTail();

          void increaseSendBufferPos(unsigned int _increaseSize);
          void decreaseSendBufferPos(unsigned int _decreaseSize);
          void resetSendBufferPos();

          /*pulse detection timeout Method*/
          void resetPulseReportedTime();
          bool isServerPulseTimeout(IN long long _timeInterval);

          template<typename T>
          void sendDataToServer(IN T* _szSendBuf, IN int _szBufferSize);

          template<typename T>
          int reciveDataFromServer(OUT T* _szRecvBuf, IN int _szBufferSize);

          /*client socket and server address*/
          SOCKET _serverSocket;                           //client connection socket
          sockaddr_in _serverAddr;

private:

          /*
          * client recive buffer(retrieve much data as possible from kernel)
          */
          const unsigned int m_szMsgBufSize = 4096;                               //4096B
          unsigned long m_szMsgPtrPos = 0;                                               //message pointer location pos
          unsigned long m_szRemainSpace = m_szMsgBufSize;                //remain space
          std::shared_ptr<char> m_szMsgBuffer;                                         //find available data from server recive buffer

          /*
          * client send buffer(create a thread and push multipule data to each client)
          */
          const unsigned int m_szSendBufSize = 4096;                                //4096B
          unsigned long m_szSendPtrPos = 0;                                               //message pointer location pos
          unsigned long m_szSendRemainSpace = m_szSendBufSize;       //remain space
          std::shared_ptr<char> m_szSendBuffer;                                        //send buffer

          /*add pulse detection timeout for each server*/
          std::chrono::time_point<std::chrono::high_resolution_clock> _lastUpdatedTime;   //last report time
};

/*------------------------------------------------------------------------------------------------------
* @function:void sendDataToServer
* @param : 1.[IN] T*_szSendBuf
                    2.[IN] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
template<typename T>
void _ServerSocket::sendDataToServer(IN T* _szSendBuf, IN int _szBufferSize)
{
          ::send(
                    this->_serverSocket,
                    reinterpret_cast<const char*>(_szSendBuf),
                    _szBufferSize,
                    0
          );
}

/*------------------------------------------------------------------------------------------------------
* @function:void reciveDataFromServer
* @param : 1. [OUT] T* _szRecvBuf
                    2. [IN OUT] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
template<typename T>
int _ServerSocket::reciveDataFromServer(OUT T* _szRecvBuf, IN int _szBufferSize)
{
          return ::recv(
                    this->_serverSocket,
                    reinterpret_cast<char*>(_szRecvBuf),
                    _szBufferSize,
                    0
          );
}

#endif // !_SERVERSOCKET_H_
