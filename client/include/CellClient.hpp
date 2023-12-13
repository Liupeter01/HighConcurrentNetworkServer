#pragma once
#ifndef _CellClient_H_
#define _CellClient_H_
#include<DataPackage.h>
#include<iostream>
#include<cassert>
#include<future>
#include<thread>

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

class CellClient
{
public:
          CellClient();
          virtual ~CellClient();

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

          SOCKET& getClientSocket();

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
          void readMessageHeader(IN _PackageHeader* _header);
          void readMessageBody(IN _PackageHeader* _buffer);

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

          /*
          * additional buffer space for 
          *  client recive buffer(retrieve much data as possible from kernel)
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

#if _WIN32 
          WSADATA m_wsadata;
#endif // _WINDOWS 
};

/*------------------------------------------------------------------------------------------------------
* @function:void sendDataToServer
* @param :
*                  1.[IN] SOCKET& _clientSocket,
                    2.[IN] T*_szSendBuf
                    3.[IN] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
template<typename T>
void CellClient::sendDataToServer(
          IN  SOCKET& _clientSocket,
          IN T* _szSendBuf,
          IN int _szBufferSize)
{
          int retvalue(SOCKET_ERROR);

          /*
          * scenario:
          *     1.isClientTransmitTimeout :
          *         true:    flushClientSendingBuffer
          *         false:  do nothing
          *
          *     2.this->getSendBufFullSpace() - this->m_szSendPtrPos <= _szBufferSize
          *           there is no space in buffer _szSendBuf, so we should send them first and clean the buffer
          *           second, we have to store the rest of the buffer
          *
          *     3.this->getSendBufFullSpace() - this->m_szSendPtrPos > _szBufferSize
          */
          while (true)
          {
                    ///*when pulse timeout(isClientTransmitTimeout)
                    // * the buffer has to be cleared and all the data should be transmit to server
                    //*/
                    //if (this->isClientTransmitTimeout()) {
                    //          this->flushClientSendingBuffer();
                    //          continue;
                    //}

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
                                        _clientSocket,
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
* @function:void reciveDataFromServer
* @param :
*                  1. IN  SOCKET& _clientSocket
                    2. [OUT] T* _szRecvBuf
                    3. [IN OUT] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
template<typename T>
int CellClient::reciveDataFromServer(
          IN  SOCKET& _clientSocket,
          OUT T* _szRecvBuf,
          IN int _szBufferSize)
{
          return ::recv(
                    _clientSocket,
                    reinterpret_cast<char*>(_szRecvBuf),
                    _szBufferSize,
                    0
          );
}

#endif 