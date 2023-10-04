#pragma once
#ifndef _CLIENTSOCKET_H_
#define _CLIENTSOCKET_H_
#include<iostream>

#if _WIN32             //Windows Enviorment  
/*break the limitaion of the select model size*/
#define FD_SETSIZE 4096      
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

class _ClientSocket {
public:
          _ClientSocket();
          _ClientSocket(
                    IN SOCKET& _socket,
                    IN sockaddr_in& _addr
          );
          virtual ~_ClientSocket();

public:
          SOCKET& getClientSocket();
          const in_addr& getClientAddr()const;
          const unsigned short& getClientPort()const;

          unsigned int getMsgPtrPos() const;
          unsigned int getBufFullSpace() const;
          unsigned int getBufRemainSpace() const;

          char* getMsgBufferHead();
          char* getMsgBufferTail();

          void increaseMsgBufferPos(unsigned int _increaseSize);
          void decreaseMsgBufferPos(unsigned int _decreaseSize);

          template<typename T> void sendDataToClient(
                    IN T* _szSendBuf,
                    IN int _szBufferSize
          );

          template<typename T> int reciveDataFromClient(
                    OUT T* _szRecvBuf,
                    IN int _szBufferSize
          );

private:
          SOCKET m_clientSocket;
          sockaddr_in m_clientAddr;

          /*
          * additional buffer space for clientDataProcessingLayer()
          * server recive buffer(retrieve much data as possible from kernel)
          */
          const unsigned int m_szMsgBufSize = 4096 * 10 ;                       //4MB
          unsigned long m_szMsgPtrPos = 0;                                               //message pointer location pos
          unsigned long m_szRemainSpace = m_szMsgBufSize;                        //remain space
          std::shared_ptr<char> m_szMsgBuffer;                                        //find available data from server recive buffer
};
#endif

/*------------------------------------------------------------------------------------------------------
* @function: void sendDataToClient
* @param : 1.[IN]  T* _szSendBuf,
                    2.[IN] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
template<typename T>
void _ClientSocket::sendDataToClient(
          IN T* _szSendBuf,
          IN int _szBufferSize)
{
          ::send(this->m_clientSocket, reinterpret_cast<const char*>(_szSendBuf), _szBufferSize, 0);
}

/*------------------------------------------------------------------------------------------------------
* @function: void reciveDataFromClient
* @param : 1. [OUT]  T* _szRecvBuf,
                    2. [IN] int &_szBufferSize

* @retvalue: int
*------------------------------------------------------------------------------------------------------*/
template<typename T>
int _ClientSocket::reciveDataFromClient(
          OUT T* _szRecvBuf,
          IN int _szBufferSize)
{
          return  ::recv(this->m_clientSocket, reinterpret_cast<char*>(_szRecvBuf), _szBufferSize, 0);
}