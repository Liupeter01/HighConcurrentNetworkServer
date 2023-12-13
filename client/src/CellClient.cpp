#include<CellClient.hpp>

CellClient::CellClient()
          :m_interfaceFuture(m_interfacePromise.get_future()),
          m_szMsgBuffer(new char[m_szMsgBufSize] {0}),
          m_szSendBuffer(new char[m_szSendBufSize] {0})
{
#if _WIN32                          //Windows Enviormen
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif
          this->m_client_socket = this->createClientSocket();
          if (this->m_client_socket == INVALID_SOCKET) {
                    return;
          }
}

CellClient::~CellClient()
{
#if _WIN32                                                   //Windows Enviorment
          ::shutdown(this->m_client_socket, SD_BOTH); //disconnect I/O
          ::closesocket(this->m_client_socket);     //release socket completely!! 
          ::WSACleanup();

#else                                                                  //Unix/Linux/Macos Enviorment
          ::shutdown(this->m_client_socket, SHUT_RDWR);//disconnect I/O and keep recv buffer
          close(this->m_client_socket);                //release socket completely!! 

#endif
}

/*------------------------------------------------------------------------------------------------------
* use socket api to create a ipv4 and tcp protocol socket 
* @function:static SOCKET createClientSocket
* @param :
*                   1.[IN] int af
*                   2.[IN] int type
*                   3.[IN] int protocol
*------------------------------------------------------------------------------------------------------*/
SOCKET CellClient::createClientSocket(
          IN int af,
          IN int type,
          IN int protocol)
{
          return  ::socket(af, type, protocol);                              //Create server socket
}

/*------------------------------------------------------------------------------------------------------
* user should input server's ip:port info to establish the connection
* @function:void connectServer
* @param : 
*                   1.[IN] unsigned long _ipAddr
*                   2.[IN] unsigned short _ipPort
*------------------------------------------------------------------------------------------------------*/
void CellClient::connectServer(
          IN unsigned long _ipAddr,
          IN unsigned short _ipPort
)
{
          this->m_server_address.sin_family = AF_INET;                                    //IPV4
          this->m_server_address.sin_port = htons(_ipPort);                                     //Port number

#if _WIN32    //Windows Enviorment
          this->m_server_address.sin_addr.S_un.S_addr = _ipAddr;                 //IP address(Windows)   
#else               /* Unix/Linux/Macos Enviorment*/
          this->m_server_address.sin_addr.s_addr = _ipAddr;                            //IP address(LINUX) 
#endif

          if (::connect(this->m_client_socket,
                    reinterpret_cast<sockaddr*>(&this->m_server_address),
                    sizeof(SOCKADDR_IN)) == SOCKET_ERROR) 
          {
                    return;
          }
}

/*------------------------------------------------------------------------------------------------------
* return the private socket
* @function:SOCKET& getClientSocket
* @retvalue: SOCKET &
*------------------------------------------------------------------------------------------------------*/
SOCKET& CellClient::getClientSocket() 
{
          return this->m_client_socket;
}

char*CellClient::getMsgBufferHead()
{
          return this->m_szMsgBuffer.get();
}

char*CellClient::getMsgBufferTail()
{
          return this->m_szMsgBuffer.get() + this->getMsgPtrPos();
}

unsigned int CellClient::getBufRemainSpace() const
{
          return this->m_szRemainSpace;
}

unsigned int CellClient::getMsgPtrPos() const
{
          return this->m_szMsgPtrPos;
}

unsigned int CellClient::getBufFullSpace() const
{
          return this->m_szMsgBufSize;
}

void CellClient::increaseMsgBufferPos(unsigned int _increaseSize)
{
          this->m_szMsgPtrPos += _increaseSize;
          this->m_szRemainSpace -= _increaseSize;
}

void CellClient::decreaseMsgBufferPos(unsigned int _decreaseSize)
{
          this->m_szMsgPtrPos -= _decreaseSize;
          this->m_szRemainSpace += _decreaseSize;
}

void CellClient::resetMsgBufferPos()
{
          this->m_szMsgPtrPos = 0;
          this->m_szRemainSpace = this->m_szMsgBufSize;
}

unsigned int CellClient::getSendPtrPos() const
{
          return this->m_szSendPtrPos;
}

unsigned int CellClient::getSendBufFullSpace() const
{
          return this->m_szSendBufSize;
}

unsigned int CellClient::getSendBufRemainSpace() const
{
          return this->m_szSendRemainSpace;
}

char*CellClient::getSendBufferHead()
{
          return this->m_szSendBuffer.get();
}

char*CellClient::getSendBufferTail()
{
          return this->m_szSendBuffer.get() + this->getSendPtrPos();
}

void CellClient::increaseSendBufferPos(unsigned int _increaseSize)
{
          this->m_szSendPtrPos += _increaseSize;
          this->m_szSendRemainSpace -= _increaseSize;
}

void CellClient::decreaseSendBufferPos(unsigned int _decreaseSize)
{
          this->m_szSendPtrPos -= _decreaseSize;
          this->m_szSendRemainSpace += _decreaseSize;
}

void CellClient::resetSendBufferPos()
{
          this->m_szSendPtrPos = 0;
          this->m_szSendRemainSpace = this->m_szSendBufSize;
}

/*------------------------------------------------------------------------------------------------------
* init IO Multiplexing
* @function: void initClientIOMultiplexing
* @description: in client, we only need to deal with client socket
*------------------------------------------------------------------------------------------------------*/
void CellClient::initClientIOMultiplexing()
{
          FD_ZERO(&m_fdread);                                                              //clean fd_read
          FD_SET(this->m_client_socket, &m_fdread);                           //Insert Server Socket into fd_read
}

/*------------------------------------------------------------------------------------------------------
* @function: bool initClientSelectModel
*------------------------------------------------------------------------------------------------------*/
bool CellClient::initClientSelectModel()
{
          return (::select(static_cast<int>(this->m_client_socket + 1),
                    &m_fdread,
                    nullptr,
                    nullptr,
                    &this->m_timeoutSetting) < 0);                  //Select Task Ended!
}

/*------------------------------------------------------------------------------------------------------
*  Currently, clientMainFunction excute on a new thread
* @function: virtual void clientInterfaceLayer
* @param: 
                    1.[IN] SOCKET & _client
                    2.[IN OUT]  std::promise<bool> &interfacePromise
*------------------------------------------------------------------------------------------------------*/
void CellClient::clientInterfaceLayer(
          IN SOCKET& _client,
          IN OUT  std::promise<bool> &interfacePromise)
{
          while (true) {
                    //char _Message[256]{ 0 };
                    //std::cin.getline(_Message, 256);
                    //if (!strcmp(_Message, "exit")) {
                    //          std::cout << "[CLIENT EXIT] Client Exit Manually" << std::endl;
                    //          interfacePromise.set_value(false);                            //set symphore value to inform other thread
                    //          return;
                    //}
                    //else if (!strcmp(_Message, "login")) {
                    //          _LoginData loginData("client-loopback404", "1234567abc");
                    //          this->sendDataToServer(_client, &loginData, sizeof(loginData));
                    //}
                    //else if (!strcmp(_Message, "logout")) {
                    //          _LogoutData logoutData("client-loopback404");
                    //          this->sendDataToServer(_client, &logoutData, sizeof(logoutData));
                    //}
                    //else {
                    //          std::cout << "[CLIENT ERROR INFO] Invalid Command Input!" << std::endl;
                    //}
                    //_LoginData loginData("client-loopback404", "1234567abc");
                    //this->sendDataToServer(_client, &loginData, sizeof(loginData));
          }
}

/*------------------------------------------------------------------------------------------------------
*  get the first sizeof(_PackageHeader) bytes of data to identify server commands
* @function: void readMessageHeader
* @param: IN _PackageHeader*
*------------------------------------------------------------------------------------------------------*/
void CellClient::readMessageHeader(IN _PackageHeader* _header)
{
          std::cout << "Receive Message From Server<Socket =" << static_cast<int>(this->m_client_socket) <<"> : "
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
* @function:bool dataProcessingLayer
*------------------------------------------------------------------------------------------------------*/
bool CellClient::dataProcessingLayer()
{
          /*enhance client data recieving capacity*/
          int  recvStatus = this->reciveDataFromServer(      //retrieve data from kernel buffer space
                    this->m_client_socket,
                    this->m_szMsgBuffer.get() + this->m_szMsgPtrPos,
                    this->m_szMsgBufSize - this->m_szMsgPtrPos
          );

          if (recvStatus <= 0) {                                             //no data recieved!
                    std::cout << "Server's Connection Terminate<Socket =" << this->m_client_socket << ","
                              << inet_ntoa(this->m_server_address.sin_addr) << ":"
                              << this->m_server_address.sin_port << ">" << std::endl;

                    return false;
          }

          this->m_szMsgPtrPos += recvStatus;                                    //get to the tail of current message

          /* judge whether the length of the data in message buffer is bigger than the sizeof(_PackageHeader) */
          while (this->m_szMsgPtrPos >= sizeof(_PackageHeader)) {
                    _PackageHeader* _header(reinterpret_cast<_PackageHeader*>(this->m_szMsgBuffer.get()));

                    /*the size of current message in szMsgBuffer is bigger than the package length(_header->_packageLength)*/
                    if (_header->_packageLength <= this->m_szMsgPtrPos) {
                              //get message header to indentify commands
                              //this->readMessageHeader(reinterpret_cast<_PackageHeader*>(_header)); 
                              //this->readMessageBody(reinterpret_cast<_PackageHeader*>(_header));

                              /*
                               * delete this message package and modify the array
                               */
#if _WIN32     //Windows Enviorment
                              memcpy_s(
                                        this->m_szMsgBuffer.get(),                                                      //the head of message buffer array
                                        this->m_szMsgBufSize,
                                        this->m_szMsgBuffer.get() + _header->_packageLength,       //the next serveral potential packages position
                                        this->m_szMsgPtrPos - _header->_packageLength                   //the size of next serveral potential package
                              );

#else               /* Unix/Linux/Macos Enviorment*/
                              memcpy(
                                        this->m_szMsgBuffer.get(),                                                      //the head of message buffer array
                                        this->m_szMsgBuffer.get() + _header->_packageLength,       //the next serveral potential packages position
                                        this->m_szMsgPtrPos - _header->_packageLength                  //the size of next serveral potential package
                              );
#endif
                              this->m_szMsgPtrPos -= _header->_packageLength;                          //recalculate the size of the rest of the array
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
          memset(this->m_szMsgBuffer.get(), 0, this->m_szMsgBufSize);
          return true;
}

/*------------------------------------------------------------------------------------------------------
* Currently, clientMainFunction only excute on the main Thread
* @function:void clientMainFunction
*------------------------------------------------------------------------------------------------------*/
void CellClient::clientMainFunction()
{
          auto res = std::async(    //startup userinput interface multithreading shared_future requires std::async to startup
                    std::launch::async,
                    &CellClient::clientInterfaceLayer,
                    this,
                    std::ref(this->m_client_socket),
                    std::ref(this->m_interfacePromise)
          );
          while (true){
                    /*wait for future variable to change (if there is no signal then ignore it and do other task)*/
                    if (this->m_interfaceFuture.wait_for(std::chrono::microseconds(1)) == std::future_status::ready) {
                              if (!this->m_interfaceFuture.get()) {
                                        break;
                              }
                    }

                    this->initClientIOMultiplexing();
                    if (this->initClientSelectModel()) {
                              break;
                    }

#if _WIN32     
                    /*CLIENT ACCELERATION PROPOSESD (Windows Enviorment only!)*/
                    if (!this->m_fdread.fd_count) {                                         //in fd_read array, no socket has been found!!               
                    }
#endif

                    if (FD_ISSET(this->m_client_socket, &this->m_fdread)) {
                              FD_CLR(this->m_client_socket, &this->m_fdread);
                              if (!this->dataProcessingLayer()) {                          //Client Exit Manually
                                        break;
                              }
                    }
          }
}