#pragma once
#ifndef _CLIENTSOCKET_H_
#define _CLIENTSOCKET_H_
#include<iostream>
#include<HCNSObjectPool.hpp>

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

/*
* inherited from HCNSObjectPool and we init 10000 _ClientSocket objects in the pool
* In order to create a objectpool, a memorypool will be created first!
*/
class _ClientSocket : public HCNSObjectPool<_ClientSocket,10000> {
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
          void resetMsgBufferPos();

          unsigned int getSendPtrPos() const;
          unsigned int getSendBufFullSpace() const;
          unsigned int getSendBufRemainSpace() const;

          char* getSendBufferHead();
          char* getSendBufferTail();

          void increaseSendBufferPos(unsigned int _increaseSize);
          void decreaseSendBufferPos(unsigned int _decreaseSize);
          void resetSendBufferPos();

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
          const unsigned int m_szMsgBufSize = 4096 ;                               //4096B
          unsigned long m_szMsgPtrPos = 0;                                               //message pointer location pos
          unsigned long m_szRemainSpace = m_szMsgBufSize;                //remain space
          std::shared_ptr<char> m_szMsgBuffer;                                         //find available data from server recive buffer

          /*
          * server send buffer(create a thread and push multipule data to each client) 
          */
          const unsigned int m_szSendBufSize = 4096;                                //4096B
          unsigned long m_szSendPtrPos = 0;                                               //message pointer location pos
          unsigned long m_szSendRemainSpace = m_szSendBufSize;       //remain space
          std::shared_ptr<char> m_szSendBuffer;                                        //send buffer
};
#endif

/*------------------------------------------------------------------------------------------------------
* @function: void sendDataToClient
* @description:all the data inside this function might not be sent immediately, all the data will be stored in the buffer space!
* @param : 1.[IN]  T* _szSendBuf,
                    2.[IN] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
template<typename T>
void _ClientSocket::sendDataToClient(IN T* _szSendBuf, IN int _szBufferSize)
{
          int retvalue(SOCKET_ERROR);

          /*
          * scenario:
          *     1.this->getSendBufFullSpace() - this->getSendPtrPos() <= _szBufferSize
          *           there is no space in buffer _szSendBuf, so we should send them first and clean the buffer
          *           second, we have to store the rest of the buffer
          *
          *     2.this->getSendBufFullSpace() - this->getSendPtrPos() > _szBufferSize
          */
          while (true)
          {
                    /* there is no space in buffer _szSendBuf, we have to send it to the client*/
                    if (this->getSendPtrPos() + _szBufferSize >= this->getSendBufFullSpace()) {

                              /* calculate valid memcpy data length */
                              int _validCopyLength = this->getSendBufFullSpace() - this->getSendPtrPos();

                              /*append _szSendBuf to the tail of the send buffer and it has to follow the constraint of _validCopyLength*/
#if _WIN32    
                              memcpy_s(
                                        reinterpret_cast<void*>(this->getSendBufferTail()),
                                        _validCopyLength,
                                        reinterpret_cast<void*>(_szSendBuf),
                                        _validCopyLength
                              );
#else        
                              memcpy(
                                        reinterpret_cast<void*>(this->getSendBufferTail()),
                                        reinterpret_cast<void*>(_szSendBuf),
                                        _validCopyLength
                              );
#endif

                              /*send all the data to client*/
                              retvalue = ::send(
                                        this->m_clientSocket,
                                        reinterpret_cast<const char*>(this->getSendBufferHead()),
                                        this->getSendBufFullSpace(), 
                                        0
                              );

                              if (retvalue == SOCKET_ERROR) {
                                        return;
                              }

                              /*
                              * move the offset of sendbuffer pointer
                              * recalculate the rest size of the sendbuffer
                              */
                              _szSendBuf = reinterpret_cast<T*>(reinterpret_cast<char*>(_szSendBuf) + _validCopyLength);
                              _szBufferSize = _szBufferSize - _validCopyLength;

                              /*reset send buffer counter*/
                              this->resetSendBufferPos();
                    }
                    else
                    {
                              /* we have enough room in the buffer, so append _szSendBuf to the tail of the send buffer */
#if _WIN32    
                              memcpy_s(
                                        reinterpret_cast<void*>(this->getSendBufferTail()),
                                        this->getSendBufRemainSpace(),
                                        reinterpret_cast<void*>(_szSendBuf),
                                        _szBufferSize
                              );
#else        
                              memcpy(
                                        reinterpret_cast<void*>(this->getSendBufferTail()),
                                        reinterpret_cast<void*>(_szSendBuf),
                                        _szBufferSize
                              );
#endif
                              this->increaseSendBufferPos(_szBufferSize);
                              break;
                    }
          }
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
