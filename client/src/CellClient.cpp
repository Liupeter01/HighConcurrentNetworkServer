#include<CellClient.hpp>

CellClient::CellClient()
          :m_interfaceFuture(m_interfacePromise.get_future()),
          _serverSocket(std::make_shared<_ServerSocket>())
{
#if _WIN32                          //Windows Enviormen
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif
          this->_serverSocket->_serverSocket = this->createClientSocket();
          if (this->_serverSocket->_serverSocket == INVALID_SOCKET) {
                    return;
          }
}

CellClient::~CellClient()
{
#if _WIN32                                                   //Windows Enviorment
          ::shutdown(this->_serverSocket->getServerSocket(), SD_BOTH); //disconnect I/O
          ::closesocket(this->_serverSocket->getServerSocket());     //release socket completely!! 
          ::WSACleanup();

#else                                                                  //Unix/Linux/Macos Enviorment
          ::shutdown(this->_serverSocket->getServerSocket(), SHUT_RDWR);//disconnect I/O and keep recv buffer
          close(this->_serverSocket->getServerSocket());                //release socket completely!! 

#endif
}

/*------------------------------------------------------------------------------------------------------
* use socket api to create a ipv4 and tcp protocol socket 
* @function:static SOCKET createClientSocket
* @param :  1.[IN] int af
*                   2.[IN] int type
*                   3.[IN] int protocol
*------------------------------------------------------------------------------------------------------*/
SOCKET CellClient::createClientSocket(IN int af, IN int type, IN int protocol)
{
          return  ::socket(af, type, protocol);                              //Create server socket
}

/*------------------------------------------------------------------------------------------------------
* user should input server's ip:port info to establish the connection
* @function:void connectToServer(IN unsigned long _ipAddr,IN unsigned short _ipPort)
* @param :  1.[IN] unsigned long _ipAddr
*                   2.[IN] unsigned short _ipPort
*------------------------------------------------------------------------------------------------------*/
void CellClient::connectToServer(IN unsigned long _ipAddr,IN unsigned short _ipPort)
{
          this->_serverSocket->_serverAddr.sin_family = AF_INET;                                    //IPV4
          this->_serverSocket->_serverAddr.sin_port = htons(_ipPort);                                     //Port number

#if _WIN32    //Windows Enviorment
          this->_serverSocket->_serverAddr.sin_addr.S_un.S_addr = _ipAddr;                 //IP address(Windows)   
#else               /* Unix/Linux/Macos Enviorment*/
          this->_serverSocket->_serverAddr.sin_addr.s_addr = _ipAddr;                            //IP address(LINUX) 
#endif

          if (::connect(this->_serverSocket->getServerSocket(),
                    reinterpret_cast<sockaddr*>(&this->_serverSocket->_serverAddr),
                    sizeof(SOCKADDR_IN)) == SOCKET_ERROR) 
          {
                    return;
          }
}

/*------------------------------------------------------------------------------------------------------
* add customized task for each server connection
* @function: void addExcuteMethod(CellClientTask&& _cellClientTask)
* @param : [IN] CellClientTask&& _cellClientTask
*------------------------------------------------------------------------------------------------------*/
void CellClient::addExcuteMethod(IN CellClientTask&& _cellClientTask)
{
          this->_func = _cellClientTask;
}

/*------------------------------------------------------------------------------------------------------
* user provide a sending buffer and buffer size in order to send message to server when sending thread started
* @function: void startExcuteMethod(IN char* _szSendBuf, IN int _szBufferSize)
* @param :  1.[IN] char* _szSendBuf
*                   2.[IN] int _szBufferSize
*------------------------------------------------------------------------------------------------------*/
void CellClient::startExcuteMethod(IN char* _szSendBuf, IN int _szBufferSize)
{
          /*add _func detection and find out wheather _func is being initalized or not*/
          if (this->_func != nullptr) {
                    this->_func(this->_serverSocket, _szSendBuf, _szBufferSize);
          }
}

/*------------------------------------------------------------------------------------------------------
* init IO Multiplexing
* @function: void initClientIOMultiplexing
* @description: in client, we only need to deal with client socket
*------------------------------------------------------------------------------------------------------*/
void CellClient::initClientIOMultiplexing()
{
          FD_ZERO(&m_fdread);                                                              //clean fd_read
          FD_SET(this->_serverSocket->getServerSocket(), &m_fdread);                           //Insert Server Socket into fd_read
}

/*------------------------------------------------------------------------------------------------------
* @function: int initClientSelectModel
*------------------------------------------------------------------------------------------------------*/
int CellClient::initClientSelectModel()
{
          return ::select(
                    static_cast<int>(this->_serverSocket->getServerSocket() + 1),
                    &m_fdread,
                    nullptr,
                    nullptr,
                    &this->m_timeoutSetting
          );
}

/*------------------------------------------------------------------------------------------------------
*  get the first sizeof(_PackageHeader) bytes of data to identify server commands
* @function: void readMessageHeader
* @param: IN _PackageHeader*
*------------------------------------------------------------------------------------------------------*/
void CellClient::readMessageHeader(IN _PackageHeader* _header)
{
          std::cout << "Receive Message From Server<Socket =" << static_cast<int>(this->_serverSocket->getServerSocket()) <<"> : "
                    << "Data Length = " << _header->_packageLength << ", Request = ";

          if (_header->_packageCmd == CMD_LOGIN) {
                    std::cout << "CMD_LOGIN, ";
          }
          else if (_header->_packageCmd == CMD_LOGOUT) {
                    std::cout << "CMD_LOGOUT, ";
          }
          else if (_header->_packageCmd == CMD_BOARDCAST) {
                    std::cout << "CMD_BOARDCAST, ";
          }
          else if (_header->_packageCmd == CMD_ERROR) {
                    std::cout << "CMD_ERROR, ";
          }
          else if (_header->_packageCmd == CMD_PULSE_DETECTION) {
                    
          }
          else {
                    std::cout << "CMD_UNKOWN" << std::endl;
          }
}

/*------------------------------------------------------------------------------------------------------
*get the first sizeof(_PackageHeader) bytes of data to identify server commands
* @function: virtual void readMessageHeader
* @param : [IN] _PackageHeader* _buffer
* ------------------------------------------------------------------------------------------------------*/
void CellClient::readMessageBody(IN _PackageHeader* _buffer)
{
          if (_buffer->_packageCmd == CMD_LOGIN) {
                    _LoginData* recvLoginData(reinterpret_cast<_LoginData*>(_buffer));
                    if (recvLoginData->loginStatus) {
                              std::cout << "username = " << recvLoginData->userName
                                        << ", userpassword = " << recvLoginData->userPassword << std::endl;
                    }
          }
          else if (_buffer->_packageCmd == CMD_LOGOUT) {
                    _LogoutData* recvLogoutData(reinterpret_cast<_LogoutData*>(_buffer));
                    if (recvLogoutData->logoutStatus) {
                              std::cout << "username = " << recvLogoutData->userName << std::endl;
                    }
          }
          else if (_buffer->_packageCmd == CMD_BOARDCAST) {
                    _BoardCast* boardcastData(reinterpret_cast<_BoardCast*>(_buffer));
                    std::cout << "New User Identification: <"
                              << boardcastData->new_ip << ":"
                              << boardcastData->new_port << ">" << std::endl;
          }
          else if (_buffer->_packageCmd == CMD_ERROR) {
                    std::cout << "Package Error: " << std::endl;
          }
}

/*------------------------------------------------------------------------------------------------------
* @function:bool serverDataProcessingLayer
* @description: use server processing layer to process data which transfer from server
* @retvalue: bool
*------------------------------------------------------------------------------------------------------*/
bool CellClient::serverDataProcessingLayer()
{
          _PackageHeader* _header(reinterpret_cast<_PackageHeader*>(
                    this->_serverSocket->getMsgBufferHead()
                    ));

          /*enhance client data recieving capacity*/
          int  recvStatus = this->_serverSocket->reciveDataFromServer(           //retrieve data from kernel buffer space
                    this->_serverSocket->getMsgBufferHead(),
                    this->_serverSocket->getSendBufRemainSpace()
          );

          if (recvStatus <= 0) {                                             //no data recieved!
                    std::cout << "Server's Connection Terminate<Socket =" << this->_serverSocket->getServerSocket() << ","
                              << inet_ntoa(this->_serverSocket->getServerAddr()) << ":"
                              << this->_serverSocket->getServerPort() << ">" << std::endl;

                    return false;
          }

          this->_serverSocket->increaseMsgBufferPos(recvStatus);      //update the pointer position 

          /* judge whether the length of the data in message buffer is bigger than the sizeof(_PackageHeader) */
          while (this->_serverSocket->getMsgPtrPos() >= sizeof(_PackageHeader)) {

                    /*the size of current message in szMsgBuffer is bigger than the package length(_header->_packageLength)*/
                    if (_header->_packageLength <= this->_serverSocket->getMsgPtrPos()) {
                              //get message header to indentify commands
                              this->readMessageHeader(reinterpret_cast<_PackageHeader*>(_header)); 
                              this->readMessageBody(reinterpret_cast<_PackageHeader*>(_header));

                              /*
                               * delete this message package and modify the array
                               */
#if _WIN32     //Windows Enviorment
                              memcpy_s(
                                        this->_serverSocket->getMsgBufferHead(),                               //the head of message buffer array
                                        this->_serverSocket->getBufFullSpace(),
                                        this->_serverSocket->getMsgBufferHead() + _header->_packageLength,       //the next serveral potential packages position
                                        this->_serverSocket->getMsgPtrPos() - _header->_packageLength                   //the size of next serveral potential package
                              );

#else               /* Unix/Linux/Macos Enviorment*/
                              memcpy(
                                        this->_serverSocket->getMsgBufferHead(),                               //the head of message buffer array
                                        this->_serverSocket->getMsgBufferHead() + _header->_packageLength,       //the next serveral potential packages position
                                        this->_serverSocket->getMsgPtrPos() - _header->_packageLength                   //the size of next serveral potential package
                              );
#endif
                              this->_serverSocket->decreaseMsgBufferPos(_header->_packageLength); //recalculate the size of the rest of the array
                    }
                    else
                    {
                              /*
                               * the size of current message in szMsgBuffer is insufficent !
                               * even can not satisfied the basic requirment of sizeof(_PackageHeader)
                               */
                              break;
                    }
          }

          /*clean the recv buffer*/
          memset(this->_serverSocket->getMsgBufferHead(), 0, this->_serverSocket->getBufFullSpace());
          return true;
}

/*------------------------------------------------------------------------------------------------------
* processing server msg(Consumer Thread)
* @function: void serverMsgProcessingThread()
*------------------------------------------------------------------------------------------------------*/
void CellClient::serverMsgProcessingThread()
{
          while (true){
                    /*wait for future variable to change (if there is no signal then ignore it and do other task)*/
                    if (this->m_interfaceFuture.wait_for(std::chrono::microseconds(1)) == std::future_status::ready) {
                              if (!this->m_interfaceFuture.get()) {
                                        break;
                              }
                    }

                    int _selectStatus = this->initClientSelectModel();
                    /*select model mulfunction*/
                    if (_selectStatus < 0) {
                              break;
                    }
                    else if (!_selectStatus) {         /*cellserver hasn't receive any data*/
                              continue;
                    }

#if _WIN32     
                    /*CLIENT ACCELERATION PROPOSESD (Windows Enviorment only!)*/
                    if (!this->m_fdread.fd_count) {                                         //in fd_read array, no socket has been found!!               
                    }
#endif

                    if (FD_ISSET(this->_serverSocket->getServerSocket(), &this->m_fdread)) {
                              FD_CLR(this->_serverSocket->getServerSocket(), &this->m_fdread);
                              if (!this->serverDataProcessingLayer()) {                          //Client Exit Manually
                                        break;
                              }
                    }
          }
}