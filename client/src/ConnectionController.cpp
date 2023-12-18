#include<ConnectionController.hpp>

ConnectionController::ConnectionController(IN long long _timeout)
          :ConnectionController(1, _timeout) 
{
}

ConnectionController::ConnectionController(IN size_t threadNumber, IN long long _timeout)
          :_reportTimeSetting(_timeout),
          m_interfaceFuture(this->m_interfacePromise.get_future())
{
          /*set ConnectionController thread number*/
          this->_threadNumber = threadNumber;

#if _WIN32                          //Windows Enviorment
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif
}

ConnectionController::~ConnectionController()
{
          this->shutdownAllConnection();
}

/*------------------------------------------------------------------------------------------------------
* use socket api to create a ipv4 and tcp protocol socket
* @function:static SOCKET createSocket
* @param :
*                   1.[IN] int af
*                   2.[IN] int type
*                   3.[IN] int protocol
*------------------------------------------------------------------------------------------------------*/
SOCKET ConnectionController::createSocket(IN int af, IN int type, IN int protocol)
{
          return  ::socket(af, type, protocol);                              //Create server socket
}

/*------------------------------------------------------------------------------------------------------
* push client's structure into a std::vector both in CellClient and _ServerSocket
* the total ammount of the clients should be the highest among other std::vector<std::shared_ptr<CellClient>::iterator
* @function: void addServerConnection
* @param: 1.[IN] unsigned long _ipAddr,
                   2.[IN] unsigned short _ipPort
*                 3.[IN] CellClientTask && _dosth
*------------------------------------------------------------------------------------------------------*/
void ConnectionController::addServerConnection(IN unsigned long _ipAddr, unsigned short _ipPort, IN CellClientTask&& _dosth)
{
          /*Create SockerAddr_IN structure and init*/
          sockaddr_in _serverAddr;
          memset(reinterpret_cast<void*>(&_serverAddr), 0, sizeof(sockaddr_in));

          _serverAddr.sin_family = AF_INET;                                    //IPV4
#if _WIN32    //Windows Enviorment
          _serverAddr.sin_addr.S_un.S_addr = _ipAddr;                 //IP address(Windows)   
#else               /* Unix/Linux/Macos Enviorment*/
          _serverAddr.sin_addr.s_addr = _ipAddr;                            //IP address(LINUX) 
#endif
          _serverAddr.sin_port = htons(_ipPort);                                     //Port number

          /*Create a Temporary _ServerSocket object*/
          std::shared_ptr<_ServerSocket> _leftServer{
                    std::make_shared<_ServerSocket>(ConnectionController::createSocket(),_serverAddr)
          };

          /*Connect to Server*/
          if (connect(_leftServer->getServerSocket(), reinterpret_cast<sockaddr*>(&_serverAddr), sizeof(SOCKADDR_IN)) == INVALID_SOCKET) {
                    return;
          }

          /*push client info into TcpServer*/
          this->m_serverInfo.emplace_back(
                    _leftServer,
                    std::forward<CellClientTask>(_dosth)
          );
}

/*------------------------------------------------------------------------------------------------------
* start to create client thread
* @function:  void connCtrlMainFunction(unsigned int _threadNumber)
* @param:[IN] unsigned int _threadNumber
*------------------------------------------------------------------------------------------------------*/
void ConnectionController::connCtrlMainFunction(unsigned int _threadNumber)
{
          if (_threadNumber <= 0) {
                    return;
          }
          
          this->_threadNumber = _threadNumber;

          /*Start an user interface for user to interrupt operation*/
          this->th_ThreadPool.emplace_back(
                    std::mem_fn(&ConnectionController::userOperateInterface), this, std::ref(this->m_interfacePromise)
          );

          /*
          *Create Cellclient object structure
          * Start each Cellclient object
          */
          for (size_t i = 0; i < this->_threadNumber; ++i) {
                    std::shared_ptr<CellClient>_leftCellClient{
                              std::make_shared<CellClient>(this->m_interfaceFuture, this, _reportTimeSetting)
                    };
                    this->m_cellClient.push_back(_leftCellClient);
          }
          std::for_each(this->m_cellClient.begin(), this->m_cellClient.end(),
                    [](std::shared_ptr<CellClient> &_cellClient) {
                              _cellClient->startCellClient();
                    }
          );

          /*find out which cellclient has the lowest client handling load*/
          std::for_each(this->m_serverInfo.begin(), this->m_serverInfo.end(),
                    [&](CellClientPackage& _server) {
                              auto lowest = findLowestConnectionLoad();                   //get load balance
                              (*lowest)->pushTemproary(
                                        _server.first,
                                        std::forward<CellClientTask>(_server.second)
                              );
                    }
          );

          while (true)
          {

          }
}

/*------------------------------------------------------------------------------------------------------
* @function: findLowestConnectionLoad()
* @retvalue : std::vector<std::shared_ptr<CellClient>>::iterator
* ------------------------------------------------------------------------------------------------------ */
std::vector<std::shared_ptr<CellClient>>::iterator  ConnectionController::findLowestConnectionLoad()
{
          auto _lowest = this->m_cellClient.begin();
          for (auto ib = this->m_cellClient.begin(); ib != this->m_cellClient.end(); ++ib) {
                    if ((*_lowest)->getConnectionLoad() > (*ib)->getConnectionLoad()) {
                              _lowest = ib;
                    }
          }
          return _lowest;
}

/*------------------------------------------------------------------------------------------------------
* @function: userOperateInterface(IN OUT std::promise<bool>& interfacePromise)
* @param : 1.[IN OUT]  std::promise<bool> &interfacePromise
* ------------------------------------------------------------------------------------------------------ */
void ConnectionController::userOperateInterface(IN OUT std::promise<bool>& interfacePromise)
{
          while (true)
          {
                    char _Message[256]{ 0 };
                    std::cin.getline(_Message, 256);
                    if (!strcmp(_Message, "exit")) {
                              std::cout << "Shutdown All Connections!" << std::endl;
                              interfacePromise.set_value(false);                            //set symphore value to inform other thread
                              return;
                    }
                    else {
                              std::cout << "Invalid Command Input!" << std::endl;
                    }
          }
}

/*------------------------------------------------------------------------------------------------------
* shutdown and terminate network connection
* @function: void purgeCloseSocket(IN typename iterator _pserver
* @param: [IN] typename  iterator _pserver
* @update: add smart pointer to control memory
*------------------------------------------------------------------------------------------------------*/
void ConnectionController::purgeCloseSocket(IN typename  iterator _pserver)
{
#if _WIN32
          ::shutdown(_pserver->first->getServerSocket(), SD_BOTH);                        //disconnect I/O
          ::closesocket(_pserver->first->getServerSocket());                                        //release socket completely!! 
#else 
          ::shutdown(_pserver->first->getServerSocket(), SHUT_RDWR);                 //disconnect I/O and keep recv buffer
          ::close(_pserver->first->getServerSocket());                                                   //release socket completely!! 
#endif
}

/*------------------------------------------------------------------------------------------------------
* shutdown ConnectionController
* @function: void shutdownAllConnection()
*------------------------------------------------------------------------------------------------------*/
void ConnectionController::shutdownAllConnection()
{
          /*shutdown all connection in client info container*/
          for (auto ib = this->m_serverInfo.begin(); ib != this->m_serverInfo.end(); ib++) {
                    this->purgeCloseSocket(ib);
          }

          /*close all the clients' connection in this cell server*/
          this->m_serverInfo.clear();

          this->m_cellClient.clear();

          std::for_each(this->th_ThreadPool.begin(), this->th_ThreadPool.end(),
                    [](std::thread& th) {
                              if (th.joinable()) {
                                        th.join();
                              }
                    }
          );

          /*cleanup wsa setup*/
#if _WIN32                          
          WSACleanup();
#endif
}

/*------------------------------------------------------------------------------------------------------
    * virtual function: server communication terminate
    * @function:  serverComTerminate(IN typename INetEvent::iteratorr _pserver)
    * @param : [IN] typename INetEvent::iterator
    * @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
    *------------------------------------------------------------------------------------------------------*/
void ConnectionController::serverComTerminate(IN typename INetEvent::iterator _pserver)
{
          std::cout << "Server Connection Lost<Socket =" << static_cast<int>(_pserver->first->getServerSocket()) << ","
                    << inet_ntoa(_pserver->first->getServerAddr()) << ":" << _pserver->first->getServerPort() << ">" << std::endl;

          /*
          * in :serverComTerminate function, the scale of the lock have to cover the all block of the code
          * in this sceniro we have to use std::vector::size() function instead of using the increment of std::vector<shared_ptr<_ServerSocket>>::iterator
          */
          std::lock_guard<std::mutex> _lckg(this->m_serverRWLock);
          auto target = std::find_if(this->m_serverInfo.begin(), this->m_serverInfo.end(),
                    [&](typename INetEvent::CellClientPackage& _server)
                    {
                              return _server.first->getServerSocket() == _pserver->first->getServerSocket();
                    }
          );
          if (target != this->m_serverInfo.end()) {
                    this->m_serverInfo.erase(target);
          }
}

/*------------------------------------------------------------------------------------------------------
  * @function:  void readMessageHeader
  * @param:  1.[IN] std::shared_ptr <_ServerSocket> _clientSocket
                      2.[IN ] _PackageHeader* _header

  * @description: process  message header
  *------------------------------------------------------------------------------------------------------*/
void ConnectionController::readMessageHeader(
          IN  std::shared_ptr <_ServerSocket> _clientSocket,
          IN _PackageHeader* _header
)
{
          std::cout << "Receive Message From Server<Socket =" << static_cast<int>(_clientSocket->getServerSocket()) << "> : "
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
  * @function:  virtual void readMessageBody
  * @param:  	1. [IN] CellClient* _cellServer,
                      2. [IN] std::shared_ptr <_ServerSocket> _clientSocket
                      3. [IN] _PackageHeader* _header

  * @description:  process  message body
  *------------------------------------------------------------------------------------------------------*/
void ConnectionController::readMessageBody(
          IN CellClient* _cellServer,
          IN std::shared_ptr <_ServerSocket> _clientSocket,
          IN _PackageHeader* _header
)
{
          if (_header->_packageCmd == CMD_LOGIN) {
                    _LoginData* recvLoginData(reinterpret_cast<_LoginData*>(_header));
                    if (recvLoginData->loginStatus) {
                              std::cout << "username = " << recvLoginData->userName
                                        << ", userpassword = " << recvLoginData->userPassword << std::endl;
                    }
          }
          else if (_header->_packageCmd == CMD_LOGOUT) {
                    _LogoutData* recvLogoutData(reinterpret_cast<_LogoutData*>(_header));
                    if (recvLogoutData->logoutStatus) {
                              std::cout << "username = " << recvLogoutData->userName << std::endl;
                    }
          }
          else if (_header->_packageCmd == CMD_BOARDCAST) {
                    _BoardCast* boardcastData(reinterpret_cast<_BoardCast*>(_header));
                    std::cout << "New User Identification: <"
                              << boardcastData->new_ip << ":"
                              << boardcastData->new_port << ">" << std::endl;
          }
          else if (_header->_packageCmd == CMD_ERROR) {
                    std::cout << "Package Error: " << std::endl;
          }
}