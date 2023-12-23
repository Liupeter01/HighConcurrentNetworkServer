#pragma once
#ifndef _CLIENTSOCKET_H_
#define _CLIENTSOCKET_H_
#include<iostream>
#include<vector>
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
* update: Add HCNSTcpServer as a friend class
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
          /*Get Client Socket and Address Info*/
          const SOCKET& getClientSocket()  const;
          const in_addr& getClientAddr()const;
          const unsigned short& getClientPort()const;

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
          bool isClientConnectionTimeout(IN long long _timeInterval);

          /*Client's Send And Receive Method*/
          template<typename T> 
          void sendDataToClient(
                    IN T* _szSendBuf,
                    IN int _szBufferSize
          );

          template<typename T> 
		  int reciveDataFromClient(
                    OUT T* _szRecvBuf,
                    IN int _szBufferSize
          );

private:
		  template<typename ClientType>
          friend class HCNSTcpServer;
		  
		  /*Release Client Socket*/
          void purgeClientSocket();

private:
          /*Client Socket and Address Info*/
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

          /*add pulse detection timeout for each client*/
          std::chrono::time_point<std::chrono::high_resolution_clock> _lastUpdatedTime;   //last report time
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
          /*Backup argument*/
          T* _szSendBackup(_szSendBuf);
          int _szSendBufSize(_szBufferSize);

          /*find out wheather _szBufferSize is bigger than 4096, and how many loops it will take.*/
          int _loop((_szBufferSize / this->getSendBufFullSpace()) + 1);

          for (int i = 0; i < _loop; ++i) {

                    int _blockSize = ((_szBufferSize - (i * this->getSendBufFullSpace())) / this->getSendBufFullSpace()) != 0 ?
                              this->getSendBufFullSpace()
                              : _szBufferSize % this->getSendBufFullSpace();

                    /* there is no space in buffer _szSendBuf, we have to send it to the client*/
                    if (this->getSendPtrPos() + _blockSize >= this->getSendBufFullSpace()) {

                              /*
                              * append _szSendBuf to the tail of the send buffer and it has to follow the constraint of getSendBufRemainSpace
                              */
#if _WIN32    
                              memcpy_s(
                                        reinterpret_cast<void*>(this->getSendBufferTail()),
                                        this->getSendBufRemainSpace(),
                                        reinterpret_cast<void*>(_szSendBackup),
                                        this->getSendBufRemainSpace()
                              );
#else        
                              memcpy(
                                        reinterpret_cast<void*>(this->getSendBufferTail()),
                                        reinterpret_cast<void*>(_szSendBackup),
                                        this->getSendBufRemainSpace()
                              );
#endif

                              /*
                              * move the offset of sendbuffer pointer
                              * recalculate the rest size of the sendbuffer
                              */
                              _szSendBackup = reinterpret_cast<T*>(reinterpret_cast<char*>(_szSendBackup) + this->getSendBufRemainSpace());
                              _szSendBufSize = _szSendBufSize - this->getSendBufRemainSpace();

                              this->increaseSendBufferPos(this->getSendBufRemainSpace());

                              /*we should send exsiting data now, and then we could deal with _szSendBuffer*/
                              int retvalue = send(
                                        this->m_clientSocket,
                                        reinterpret_cast<const char*>(this->getSendBufferHead()),
                                        this->getSendPtrPos(),
                                        0
                              );

                              if (retvalue == SOCKET_ERROR) {
                                        return;
                              }


                              /*reset send buffer counter*/
                              this->resetSendBufferPos();

                    }
                    else {
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
