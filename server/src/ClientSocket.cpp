#include "ClientSocket.h"

_ClientSocket::_ClientSocket()
          :m_clientSocket(INVALID_SOCKET)
{
          memset(&this->m_clientAddr, 0, sizeof(sockaddr_in));
}

_ClientSocket::_ClientSocket(
          IN SOCKET& _socket,
          IN sockaddr_in& _addr)
          :m_clientSocket(_socket),
          m_szMsgBuffer(new char[m_szMsgBufSize] {0})
{
          ::memcpy(
                    reinterpret_cast<void*>(&this->m_clientAddr),
                    reinterpret_cast<const void*>(&_addr),
                    sizeof(sockaddr_in)
          );
}

SOCKET& _ClientSocket::getClientSocket()
{
          return this->m_clientSocket;
}

const in_addr& _ClientSocket::getClientAddr() const
{
          return this->m_clientAddr.sin_addr;
}

const unsigned short& _ClientSocket::getClientPort() const
{
          return this->m_clientAddr.sin_port;
}

char* _ClientSocket::getMsgBufferHead()
{
          return this->m_szMsgBuffer.get();
}

char* _ClientSocket::getMsgBufferTail()
{
          return this->m_szMsgBuffer.get() + this->getMsgPtrPos();
}

unsigned int _ClientSocket::getBufRemainSpace() const
{
          return this->m_szRemainSpace;
}

unsigned int _ClientSocket::getMsgPtrPos() const
{
          return this->m_szMsgPtrPos;
}

unsigned int _ClientSocket::getBufFullSpace() const
{
          return this->m_szMsgBufSize;
}

void  _ClientSocket::increaseMsgBufferPos(unsigned int _increaseSize)
{
          this->m_szMsgPtrPos += _increaseSize;
          this->m_szRemainSpace -= _increaseSize;
}

void  _ClientSocket::decreaseMsgBufferPos(unsigned int _decreaseSize)
{
          this->m_szMsgPtrPos -= _decreaseSize;
          this->m_szRemainSpace += _decreaseSize;
}

_ClientSocket::~_ClientSocket()
{
          //[POTIENTAL BUG HERE!]: why _clientaddr's dtor was deployed
         // ::closesocket(this->m_clientSocket);
          this->m_clientSocket = INVALID_SOCKET;
          memset(
                    reinterpret_cast<void*>(&this->m_clientAddr),
                    0,
                    sizeof(sockaddr_in)
          );
}
