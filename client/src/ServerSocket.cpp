#include<ServerSocket.hpp>

_ServerSocket::_ServerSocket()
          :_serverSocket(INVALID_SOCKET)
{
          memset(reinterpret_cast<void*>(&this->_serverAddr), 0, sizeof(sockaddr_in));
}

_ServerSocket::_ServerSocket(IN SOCKET& _socket, IN sockaddr_in& _addr)
          :_serverSocket(_socket),
          m_szMsgBuffer(new char[m_szMsgBufSize] {0}),
          m_szSendBuffer(new char[m_szSendBufSize] {0})
{
          ::memcpy(
                    reinterpret_cast<void*>(&this->_serverAddr),
                    reinterpret_cast<const void*>(&_addr),
                    sizeof(sockaddr_in)
          );

          /*reset timeout*/
          this->resetPulseReportedTime();
}

_ServerSocket:: ~_ServerSocket()
{
          this->_serverSocket = INVALID_SOCKET;
          memset(reinterpret_cast<void*>(&this->_serverAddr), 0, sizeof(sockaddr_in));
}

SOCKET& _ServerSocket::getServerSocket()
{
          return this->_serverSocket;
}

const in_addr& _ServerSocket::getServerAddr()const
{
          return this->_serverAddr.sin_addr;
}
const unsigned short& _ServerSocket::getServerPort()const
{
          return this->_serverAddr.sin_port;
}

char* _ServerSocket::getMsgBufferHead()
{
          return this->m_szMsgBuffer.get();
}

char* _ServerSocket::getMsgBufferTail()
{
          return this->m_szMsgBuffer.get() + this->getMsgPtrPos();
}

unsigned int _ServerSocket::getBufRemainSpace() const
{
          return this->m_szRemainSpace;
}

unsigned int _ServerSocket::getMsgPtrPos() const
{
          return this->m_szMsgPtrPos;
}

unsigned int _ServerSocket::getBufFullSpace() const
{
          return this->m_szMsgBufSize;
}

void  _ServerSocket::increaseMsgBufferPos(unsigned int _increaseSize)
{
          this->m_szMsgPtrPos += _increaseSize;
          this->m_szRemainSpace -= _increaseSize;
}

void  _ServerSocket::decreaseMsgBufferPos(unsigned int _decreaseSize)
{
          this->m_szMsgPtrPos -= _decreaseSize;
          this->m_szRemainSpace += _decreaseSize;
}

void _ServerSocket::resetMsgBufferPos()
{
          this->m_szMsgPtrPos = 0;
          this->m_szRemainSpace = this->m_szMsgBufSize;
}

unsigned int _ServerSocket::getSendPtrPos() const
{
          return this->m_szSendPtrPos;
}

unsigned int _ServerSocket::getSendBufFullSpace() const
{
          return this->m_szSendBufSize;
}

unsigned int _ServerSocket::getSendBufRemainSpace() const
{
          return this->m_szSendRemainSpace;
}

char* _ServerSocket::getSendBufferHead()
{
          return this->m_szSendBuffer.get();
}

char* _ServerSocket::getSendBufferTail()
{
          return this->m_szSendBuffer.get() + this->getSendPtrPos();
}

void _ServerSocket::increaseSendBufferPos(unsigned int _increaseSize)
{
          this->m_szSendPtrPos += _increaseSize;
          this->m_szSendRemainSpace -= _increaseSize;
}

void _ServerSocket::decreaseSendBufferPos(unsigned int _decreaseSize)
{
          this->m_szSendPtrPos -= _decreaseSize;
          this->m_szSendRemainSpace += _decreaseSize;
}

void _ServerSocket::resetSendBufferPos()
{
          this->m_szSendPtrPos = 0;
          this->m_szSendRemainSpace = this->m_szSendBufSize;
}

/*------------------------------------------------------------------------------------------------------
* @function: void resetPulseReportedTime()
* @description: update last report time to class argument by current time
*------------------------------------------------------------------------------------------------------*/
void _ServerSocket::resetPulseReportedTime()
{
          this->_lastUpdatedTime = std::chrono::high_resolution_clock::now();
}

/*------------------------------------------------------------------------------------------------------
* @function: bool isServerPulseTimeout(IN long long _timeInterval)
* @description: check is server going to terminate connection
* @retvalue: [IN] long long _timeInterval
* @retvalue: bool
*------------------------------------------------------------------------------------------------------*/
bool _ServerSocket::isServerPulseTimeout(IN long long _timeInterval)
{
          auto _time_interval = std::chrono::high_resolution_clock::now() - this->_lastUpdatedTime;
          return (std::chrono::duration_cast<std::chrono::milliseconds>(_time_interval).count() <= _timeInterval);
}