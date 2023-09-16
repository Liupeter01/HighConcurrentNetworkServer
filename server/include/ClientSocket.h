#pragma once
#ifndef _CLIENTSOCKET_H_
#define _CLIENTSOCKET_H_
#include<iostream>

#if _WIN32             //Windows Enviorment  
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

private:
          SOCKET m_clientSocket;
          sockaddr_in m_clientAddr;

          const unsigned int m_szMsgBufSize = 4096 * 10 ;                       //100KB
          unsigned long m_szMsgPtrPos = 0;                                               //message pointer location pos
          unsigned long m_szRemainSpace = m_szMsgBufSize;                        //remain space
          std::shared_ptr<char> m_szMsgBuffer;                                        //find available data from server recive buffer
};
#endif