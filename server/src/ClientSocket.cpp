#include <ClientSocket.hpp>

_ClientSocket::_ClientSocket()
          :m_clientSocket(INVALID_SOCKET)
{
          memset(&this->m_clientAddr, 0, sizeof(sockaddr_in));
}

_ClientSocket::_ClientSocket(
          IN SOCKET& _socket,
          IN sockaddr_in& _addr)
          :m_clientSocket(_socket),
          m_szMsgBuffer(new char[m_szMsgBufSize] {0}),
          m_szSendBuffer(new char[m_szSendBufSize] {0})
{
          ::memcpy(
                    reinterpret_cast<void*>(&this->m_clientAddr),
                    reinterpret_cast<const void*>(&_addr),
                    sizeof(sockaddr_in)
          );

          /*reset timeout*/
          this->resetPulseReportedTime();
}

_ClientSocket::~_ClientSocket()
{
          //[POTIENTAL BUG HERE!]: why _clientaddr's dtor was deployed
          this->purgeClientSocket();
}

const 
SOCKET& 
_ClientSocket::getClientSocket()  const
{
          return this->m_clientSocket;
}

const 
in_addr& 
_ClientSocket::getClientAddr() const
{
          return this->m_clientAddr.sin_addr;
}

const 
unsigned short&
_ClientSocket::getClientPort() const
{
          return this->m_clientAddr.sin_port;
}

char* 
_ClientSocket::getMsgBufferHead()
{
          return this->m_szMsgBuffer.get();
}

char* 
_ClientSocket::getMsgBufferTail()
{
          return this->m_szMsgBuffer.get() + this->getMsgPtrPos();
}

const
unsigned int
_ClientSocket::getBufRemainSpace() const
{
          return this->m_szRemainSpace;
}

const
unsigned int
_ClientSocket::getMsgPtrPos() const
{
          return this->m_szMsgPtrPos;
}

const
unsigned int
_ClientSocket::getBufFullSpace() const
{
          return this->m_szMsgBufSize;
}

void  
_ClientSocket::increaseMsgBufferPos(unsigned int _increaseSize)
{
          this->m_szMsgPtrPos += _increaseSize;
          this->m_szRemainSpace -= _increaseSize;
}

void  
_ClientSocket::decreaseMsgBufferPos(unsigned int _decreaseSize)
{
          this->m_szMsgPtrPos -= _decreaseSize;
          this->m_szRemainSpace += _decreaseSize;
}

void 
_ClientSocket::resetMsgBufferPos()
{
          this->m_szMsgPtrPos = 0;
          this->m_szRemainSpace = this->m_szMsgBufSize;
}

const
unsigned int
_ClientSocket::getSendPtrPos() const
{
          return this->m_szSendPtrPos;
}

const
unsigned int
_ClientSocket::getSendBufFullSpace() const
{
          return this->m_szSendBufSize;
}

const
unsigned int
_ClientSocket::getSendBufRemainSpace() const
{
          return this->m_szSendRemainSpace;
}

char* 
_ClientSocket::getSendBufferHead()
{
          return this->m_szSendBuffer.get();
}

char* 
_ClientSocket::getSendBufferTail()
{
          return this->m_szSendBuffer.get() + this->getSendPtrPos();
}

void 
_ClientSocket::increaseSendBufferPos(unsigned int _increaseSize)
{
          this->m_szSendPtrPos += _increaseSize;
          this->m_szSendRemainSpace -= _increaseSize;
}

void 
_ClientSocket::decreaseSendBufferPos(unsigned int _decreaseSize)
{
          this->m_szSendPtrPos -= _decreaseSize;
          this->m_szSendRemainSpace += _decreaseSize;
}

void 
_ClientSocket::resetSendBufferPos()
{
          this->m_szSendPtrPos = 0;
          this->m_szSendRemainSpace = this->m_szSendBufSize;
}

/*------------------------------------------------------------------------------------------------------
* this function should ONLY called by HCNSTcpServer!
* @function: void purgeClientSocket()
* @description: 1.close ClientSocket and clean client's IP addr
				2.clean clientSocket value to INVALID_SOCKET
*------------------------------------------------------------------------------------------------------*/
void  
_ClientSocket::purgeClientSocket()
{
          /*Add valid socket condition*/
          if (INVALID_SOCKET != this->m_clientSocket) {
#if _WIN32                                                   //Windows Enviorment
                    ::shutdown(this->m_clientSocket, SD_BOTH); //disconnect I/O
                    ::closesocket(this->m_clientSocket);     //release socket completely!! 

#else                                                                  //Unix/Linux/Macos Enviorment
                    ::shutdown(this->m_clientSocket, SHUT_RDWR);//disconnect I/O and keep recv buffer
                    close(this->m_clientSocket);                //release socket completely!! 

#endif

                    /*set memory value*/
                    memset(reinterpret_cast<void*>(&this->m_clientAddr), 0, sizeof(sockaddr_in));

                    /*set socket to INVALID_SOCKET*/
                    this->m_clientSocket = INVALID_SOCKET;
          }
}

/*------------------------------------------------------------------------------------------------------
* @function: void resetPulseReportedTime()
* @description: update last report time to class argument by current time
*------------------------------------------------------------------------------------------------------*/
void 
_ClientSocket::resetPulseReportedTime()
{
          this->_lastUpdatedTime = std::chrono::high_resolution_clock::now();
}

/*------------------------------------------------------------------------------------------------------
* @function: bool isClientConnectionTimeout(long long _timeInterval)
* @description: check is this client connection timeout(over specific time without reply)
* @retvalue: [IN] long long _timeInterval
* @retvalue: bool
*------------------------------------------------------------------------------------------------------*/
bool 
_ClientSocket::isClientConnectionTimeout(IN long long _timeInterval)
{
          auto _time_interval = std::chrono::high_resolution_clock::now() - this->_lastUpdatedTime;
          return (std::chrono::duration_cast<std::chrono::milliseconds>(_time_interval).count() <= _timeInterval);
}