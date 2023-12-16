#pragma once
#ifndef _HCNSTCPSERVER_H_
#define _HCNSTCPSERVER_H_
#include<HCNSCellServer.hpp>

template<class ClientType = _ClientSocket>
class HCNSTcpServer :public INetEvent<ClientType>
{
public:
          HCNSTcpServer();
          HCNSTcpServer(IN unsigned short _ipPort, IN long long _timeout);
          HCNSTcpServer(IN unsigned long _ipAddr, IN unsigned short _ipPort, IN long long _timeout);
          virtual ~HCNSTcpServer();

public:
          static SOCKET createServerSocket(
                    IN int af = AF_INET,
                    IN int type = SOCK_STREAM,
                    IN int protocol = IPPROTO_TCP
          );

          static bool acceptClientConnection(
                    IN SOCKET serverSocket,
                    OUT SOCKET* clientSocket,
                    OUT SOCKADDR_IN* _clientAddr
          );

public:
          void serverMainFunction(IN const unsigned int _threadNumber);

protected:
          int startServerListening(IN int backlog = SOMAXCONN);

private:
          void purgeCloseSocket(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient);
          void initServerAddressBinding(unsigned long _ipAddr,unsigned short _port);

          bool initServerSelectModel(IN SOCKET _largestSocket);
          void serverInterfaceLayer(IN OUT std::promise<bool>& interfacePromise);

          void pushClientToCellServer(IN SOCKET &_clientSocket,IN sockaddr_in &_clientAddress);
          void clientConnectionThread();

          void getClientsUploadSpeed();

          void shutdownTcpServer();

private:
          virtual inline void clientOnJoin(IN std::shared_ptr<ClientType> _pclient);
          virtual void clientOnLeave(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient);
          virtual void addUpRecvCounter(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient);


          virtual inline void addUpClientsCounter();
          virtual inline void decreaseClientsCounter();
          virtual inline void addUpPackageCounter();
          virtual inline void readMessageHeader(
                    IN  std::shared_ptr <ClientType> _clientSocket,
                    IN _PackageHeader* _header
          );

          virtual inline void readMessageBody(
                    IN HCNSCellServer<ClientType>* _cellServer,
                    IN std::shared_ptr<ClientType> _clientSocket,
                    IN _PackageHeader* _header
          );

private:
          /*Client Pulse Timeout Setting*/
          long long _reportTimeSetting;

          /*server interface symphoare control and thread creation*/
          std::promise<bool> m_interfacePromise;
          std::shared_future<bool> m_interfaceFuture;

          /*
          * tcpserver threadpool
          * 1.interfaceThread
          * 2.server clientConnection Thread(Producer)
          */
          std::vector<std::thread> th_tcpServerThreadPool;

          /*server socket info*/
          SOCKET m_server_socket;                           //server listening socket
          sockaddr_in m_server_address;

          /*select network model*/
          fd_set m_fdread;
          fd_set m_fdwrite;
          fd_set m_fdexception;
          timeval m_timeoutSetting{ 0/*0 s*/, 0 /*0 ms*/ };

          /*
          * std::shared_ptr<HCNSCellServer> (Consumer Thread)
          * add memory smart pointer to control memory allocation
          */
          std::vector<std::shared_ptr<HCNSCellServer<ClientType>>> m_cellServer;

          /*
          * std::shared_ptr<ClientType>
          * HCNSCellServer should record all the connection
          * add memory smart pointer to record all the connection
          */
          std::mutex m_clientRWLock;
           std::vector< std::shared_ptr<ClientType>> m_clientInfo;

          /*record recv function calls*/
          std::atomic<unsigned long long> m_recvCounter;

          /*record packages received in HCNSCellServer container*/
          std::atomic<unsigned long long> m_packageCounter;

          /*record clients connected to HCNSCellServer container*/
          std::atomic<unsigned long long> m_ClientsCounter;

          /*
          * Server High Resolution Clock Model(How Many Packages are recieved per second)
          * origin def:  HCNSTimeStamp* m_timeStamp;
          */
          std::shared_ptr<HCNSTimeStamp> m_timeStamp;

#if _WIN32 
          WSADATA m_wsadata;
#endif // _WINDOWS 
};
#endif

template<class ClientType>
HCNSTcpServer<ClientType>::HCNSTcpServer()
          :m_server_address{ 0 },
          m_server_socket(INVALID_SOCKET)
{
}

template<class ClientType>
HCNSTcpServer<ClientType>::HCNSTcpServer(IN unsigned short _ipPort, IN long long _timeout)
          :HCNSTcpServer(INADDR_ANY, _ipPort, _timeout)
{
}

template<class ClientType>
HCNSTcpServer<ClientType>::HCNSTcpServer( IN unsigned long _ipAddr,  IN unsigned short _ipPort, IN long long _timeout)
          : m_timeStamp(std::make_shared<HCNSTimeStamp>()),
          m_interfaceFuture(this->m_interfacePromise.get_future()),
          m_packageCounter(0),
          m_recvCounter(0),
          m_ClientsCounter(0),
          _reportTimeSetting(_timeout)
{
#if _WIN32                          //Windows Enviorment
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif
          this->initServerAddressBinding(_ipAddr, _ipPort);

          /*Start Server Listening and Setup Listening Queue Number = SOMAXCONN*/
          this->startServerListening();
}

template<class ClientType>
HCNSTcpServer<ClientType>::~HCNSTcpServer() 
{
          this->shutdownTcpServer();
}

/*------------------------------------------------------------------------------------------------------
* use socket api to create a ipv4 and tcp protocol socket
* @function: static SOCKET createServerSocket
* @param :  1.[IN] int af
*                   2.[IN] int type
*                   3.[IN] int protocol
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
SOCKET HCNSTcpServer<ClientType>::createServerSocket(IN int af, IN int type, IN int protocol)
{
          return ::socket(af, type, protocol);                              //Create server socket
}

/*------------------------------------------------------------------------------------------------------
* use accept function to accept connection which come from client
* @function: static bool acceptClientConnection
* @param :
*                   1.[IN] SOCKET  serverSocket
*                   3.[OUT] SOCKET *clientSocket,
*                   2.[OUT] SOCKADDR_IN* _clientAddr
*
* @retvalue: bool
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
bool HCNSTcpServer<ClientType>::acceptClientConnection(
          IN SOCKET serverSocket,
          OUT SOCKET* clientSocket,
          OUT SOCKADDR_IN* _clientAddr)
{
#if _WIN32             //Windows Enviorment
          int addrlen = sizeof(SOCKADDR_IN);
#else                         /* Unix/Linux/Macos Enviorment*/
          socklen_t addrlen = sizeof(SOCKADDR_IN);
#endif  

          if ((*clientSocket = ::accept(serverSocket, reinterpret_cast<sockaddr*>(_clientAddr), &addrlen)) == INVALID_SOCKET) {                 //socket error occured!
                    return false;
          }
          return true;
}

/*------------------------------------------------------------------------------------------------------
* start server listening
* @function:  int startServerListening
* @param : int backlog
* @retvalue : int
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
int HCNSTcpServer<ClientType>::startServerListening(IN int backlog)
{
          return ::listen(this->m_server_socket, backlog);
}

/*------------------------------------------------------------------------------------------------------
* start to create server thread
* @function:  void serverMainFunction(IN const unsigned int _threadNumber)
* @param [IN] IN const unsigned int _threadNumber
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::serverMainFunction(IN const unsigned int _threadNumber)
{
          if (_threadNumber <= 0) {
                    std::cout << "Invalid Server Thread Number" << std::endl;
                    return;
          }
          th_tcpServerThreadPool.emplace_back(
                    std::mem_fn(&HCNSTcpServer::clientConnectionThread), this
          );
          th_tcpServerThreadPool.emplace_back(
                    std::mem_fn(&HCNSTcpServer::serverInterfaceLayer), this, std::ref(this->m_interfacePromise)
          );

          for (size_t i = 0; i < _threadNumber; ++i) {
                    std::shared_ptr<HCNSCellServer<ClientType>>_leftCellServer(
                              new HCNSCellServer<ClientType>(
                                        this->m_server_socket,
                                        this->m_server_address,
                                        this->m_interfaceFuture,
                                        this,
                                        this->_reportTimeSetting
                              )
                    );
                    this->m_cellServer.push_back(std::move(_leftCellServer));
          }
          std::for_each(this->m_cellServer.begin(), this->m_cellServer.end(),
                    [](std::shared_ptr<HCNSCellServer<ClientType>>& _cellServer){
                              _cellServer->startCellServer();
                    }
          );
}

/*------------------------------------------------------------------------------------------------------
* shutdown and terminate network connection
* @function: void pushTemproaryClient(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient)
* @param: [IN] typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient
* @update: add smart pointer to control memory
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::purgeCloseSocket(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient)
{
          this->decreaseClientsCounter();
#if _WIN32
          ::shutdown((*_pclient)->getClientSocket(), SD_BOTH);                        //disconnect I/O
          ::closesocket((*_pclient)->getClientSocket());                                        //release socket completely!! 
#else 
          ::shutdown((*_pclient)->getClientSocket(), SHUT_RDWR);                 //disconnect I/O and keep recv buffer
          ::close((*_pclient)->getClientSocket());                                                   //release socket completely!! 
#endif
}

/*------------------------------------------------------------------------------------------------------
* init server ip address and port and bind it with server socket
* @function: void initServerAddressBinding
* @implementation: create server socket and bind ip address info
* @param : 1.[IN] unsigned long _ipAddr
           2.[IN] unsigned short _port
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::initServerAddressBinding(unsigned long _ipAddr,unsigned short _port)
{
          if ((this->m_server_socket = createServerSocket()) == INVALID_SOCKET) {
                    return;
          }
          this->m_server_address.sin_family = AF_INET;                                    //IPV4
          this->m_server_address.sin_port = htons(_port);                                     //Port number

#if _WIN32     //Windows Enviorment
          this->m_server_address.sin_addr.S_un.S_addr = _ipAddr;                 //IP address(Windows)   
#else               /* Unix/Linux/Macos Enviorment*/
          this->m_server_address.sin_addr.s_addr = _ipAddr;                            //IP address(LINUX) 
#endif

          if (::bind(this->m_server_socket, reinterpret_cast<sockaddr*>(&this->m_server_address), sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
                    return;
          }
}

/*------------------------------------------------------------------------------------------------------
* use select system call to setup model
* @function: bool initServerSelectModel(SOCKET _largestSocket)
* @param: SOCKET _largestSocket
* @retvalue: bool
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
bool HCNSTcpServer<ClientType>::initServerSelectModel(IN SOCKET _largestSocket)
{
          return (
                    ::select(
                              static_cast<int>(_largestSocket) + 1,
                              &this->m_fdread,
                              nullptr,
                              nullptr,
                              &this->m_timeoutSetting
                    ) < 0
          );
}

/*------------------------------------------------------------------------------------------------------
* Currently, serverMainFunction excute on a new thread
* @function: virtual void serverInterfaceLayer
* @param : 1.[IN OUT]  std::promise<bool> &interfacePromise
* ------------------------------------------------------------------------------------------------------ */
template<class ClientType>
void HCNSTcpServer<ClientType>::serverInterfaceLayer(IN OUT std::promise<bool>& interfacePromise)
{
          while (true)
          {
                    char _Message[256]{ 0 };
                    std::cin.getline(_Message, 256);
                    if (!strcmp(_Message, "exit")) {
                              std::cout << "Server Interrupted By User, All Connections Close!!" << std::endl;
                              interfacePromise.set_value(false);                            //set symphore value to inform other thread
                              return;
                    }
                    else {
                              std::cout << "Server Invalid Command Input!" << std::endl;
                    }
          }
}

/*------------------------------------------------------------------------------------------------------
* push client's structure into a std::vector both in HCNSCellServer and ClientSocket
* the total ammount of the clients should be the highest among other  std::vector<std::shared_ptr<HCNSCellServer>>::iterator
* @function: void :pushClientToCellServer(IN SOCKET &_clientSocket,IN sockaddr_in &_clientAddress)
* @param: 1.[IN] SOCKET &_clientSocket
                   2.[IN] IN sockaddr_in &_clientAddress

*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::pushClientToCellServer(IN SOCKET &_clientSocket,IN sockaddr_in &_clientAddress)
{
          /*Create a smart pointer for ClientType*/
          std::shared_ptr<ClientType> _leftClientInfo{
            new ClientType(_clientSocket, _clientAddress)
          };

          /*find out which cell server has the lowest client handling load*/
          auto _lowest = this->m_cellServer.begin();
          for (auto ib = this->m_cellServer.begin(); ib != this->m_cellServer.end(); ++ib) {
                    if ((*_lowest)->getClientsConnectionLoad() > (*ib)->getClientsConnectionLoad()) {
                              _lowest = ib;
                    }
          }

          {
                    std::lock_guard<std::mutex> _lckg(this->m_clientRWLock);
                    /*push client info into TcpServer*/
                    this->m_clientInfo.push_back(_leftClientInfo);

                    /*client join the server*/
                    this->clientOnJoin(_leftClientInfo);

                    /*push clientinfo into the specific CellServer*/
                    (*_lowest)->pushTemproaryClient(_leftClientInfo);
          }
}

/*------------------------------------------------------------------------------------------------------
* Producer Thread: Accept Clients Connection and put it into the queue and wait for Comsumer
* @function: void clientConnectionThread
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::clientConnectionThread()
{
          while (true) 
          {            
                    /*init tcpserver select model*/
                    FD_ZERO(&this->m_fdread);                                                               //clean fd_read
                    FD_SET(this->m_server_socket, &this->m_fdread);                           //Insert Server Socket into fd_read

                    /*the number of server socket is the largest in client connection thread(producer)*/
                    if (this->initServerSelectModel(this->m_server_socket)) {
                              this->shutdownTcpServer();                 //when select model fails then shutdown server
                              break;
                    }

                    /*calculate the clients upload speed, according to all cell servers*/
                    this->getClientsUploadSpeed();

                    if (FD_ISSET(this->m_server_socket, &m_fdread)) {    //Detect client message input signal
                              SOCKET _clientSocket;                                       //the temp variable to record client's socket
                              sockaddr_in _clientAddress;                                 //the temp variable to record client's address                                     
                              FD_CLR(this->m_server_socket, &m_fdread);      //delete client message signal

                              this->acceptClientConnection(                               //
                                        this->m_server_socket,
                                        &_clientSocket,
                                        &_clientAddress
                              );

                              /* put client's connection stucture to the back of the queue*/
                              this->pushClientToCellServer(_clientSocket, _clientAddress);
                    }
          }
}

/*------------------------------------------------------------------------------------------------------
* calculate the clients upload speed, according to all cell servers
* @function: void getClientsUploadSpeed()
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::getClientsUploadSpeed()
{
          if (this->m_timeStamp->getElaspsedTimeInsecond() >= 1LL){

                    std::cout << "[" << this->m_timeStamp->printCurrentTime() << "]: "
                              << "<Connections:" << this->m_ClientsCounter << "> "
                              << "Client's Upload Speed = "
                              << this->m_packageCounter / this->m_timeStamp->getElaspsedTimeInsecond() << " Packages/s"
                              << " Recv Function Called Frequency = "
                              << this->m_recvCounter / this->m_timeStamp->getElaspsedTimeInsecond() << " Times/s"
                              << std::endl;

                    /*reset package counter*/
                    this->m_packageCounter = 0;

                    /*reset recv function call times*/
                    this->m_recvCounter = 0;

                    /*reset timer*/
                    this->m_timeStamp->updateTimer();
          }
}

/*------------------------------------------------------------------------------------------------------
* shutdown tcp server
* @function: void shutdownTcpServer
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::shutdownTcpServer()
{
          /*shutdown all the clients' connection in client info container*/
          for (auto ib = this->m_clientInfo.begin(); ib != this->m_clientInfo.end(); ib++) {
                    this->purgeCloseSocket(ib);
          }
          this->m_clientInfo.clear();

          /*close all the clients' connection in this cell server*/
          this->m_cellServer.clear(); 

          std::for_each(this->th_tcpServerThreadPool.begin(), this->th_tcpServerThreadPool.end(),
                    [](std::thread& th) {
                              if (th.joinable()) {
                                        th.join();
                              }
                    }
          );

#if _WIN32             
          ::shutdown(this->m_server_socket, SD_BOTH);                     //disconnect I/O
          ::closesocket(this->m_server_socket);                                      //release socket completely!! 
#else                                   
          ::shutdown(this->m_server_socket, SHUT_RDWR);               //disconnect I/O and keep recv buffer
          close(this->m_server_socket);                                                   //release socket completely!! 
#endif

          /*reset socket*/
          this->m_server_socket = INVALID_SOCKET;

          /*cleanup wsa setup*/
#if _WIN32                          
          WSACleanup();
#endif
}

/*------------------------------------------------------------------------------------------------------
  * virtual function: client connect to server
  * @function:  void clientOnJoin(IN std::shared_ptr<ClientType> _pclient)
  * @param : [IN] std::shared_ptr<ClientType> _pclient
  *------------------------------------------------------------------------------------------------------*/
template<class ClientType>
inline void HCNSTcpServer<ClientType>::clientOnJoin(IN std::shared_ptr<ClientType> _pclient)
{
          this->addUpClientsCounter();

          std::cout << "Accept Client's Connection<Socket =" << static_cast<int>(_pclient->getClientSocket()) << ","
                    << inet_ntoa(_pclient->getClientAddr()) << ":" << _pclient->getClientPort() << ">" << std::endl;
}

/*------------------------------------------------------------------------------------------------------
* virtual function: client terminate connection
* @function:  void clientOnLeave(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient)
* @param : [IN] typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient
* @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::clientOnLeave(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient)
{
          /* decrease m_clientCounter */
          this->decreaseClientsCounter();

          std::cout << "Terminate Client's Connection<Socket =" << static_cast<int>((*_pclient)->getClientSocket()) << ","
                    << inet_ntoa((*_pclient)->getClientAddr()) << ":" << (*_pclient)->getClientPort() << ">" << std::endl;

          /*
          * in clientOnLeave function, the scale of the lock have to cover the all block of the code
          * in this sceniro we have to use std::vector::size() function instead of using the increment of std::vector<shared_ptr<ClientType>>::iterator
          */
          std::lock_guard<std::mutex> _lckg(this->m_clientRWLock);
          auto target = std::find_if(this->m_clientInfo.begin(), this->m_clientInfo.end(),
                    [&](std::shared_ptr<ClientType> _client)
                    {
                              return _client->getClientSocket() == (*_pclient)->getClientSocket();
                    }
          );
          if (target != this->m_clientInfo.end()) {
                    this->m_clientInfo.erase(target);
          }
}

/*------------------------------------------------------------------------------------------------------
 * virtual function: add up to the number of recv function calls
 * @function:  void addUpRecvCounter(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient)
 * @param : [IN] typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient
 * @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
 *------------------------------------------------------------------------------------------------------*/
template<class ClientType>
inline void HCNSTcpServer<ClientType>::addUpRecvCounter(IN typename  std::vector< std::shared_ptr<ClientType>>::iterator _pclient)
{
          this->m_recvCounter++;
}

/*------------------------------------------------------------------------------------------------------
* virtual function: add up to the number of clients
  * @function:  void addUpClientsCounter()
  * @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
  *------------------------------------------------------------------------------------------------------*/
template<class ClientType>
inline void HCNSTcpServer<ClientType>::addUpClientsCounter()
{
          ++this->m_ClientsCounter;
}

/*------------------------------------------------------------------------------------------------------
  * virtual function: decrease to the number of clients
  * @function:  decreaseClientsCounter()
  * @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
  *------------------------------------------------------------------------------------------------------*/
template<class ClientType>
inline void HCNSTcpServer<ClientType>::decreaseClientsCounter()
{
          --this->m_ClientsCounter;
}

/*------------------------------------------------------------------------------------------------------
  * virtual function: add up to the number of packages being received
  * @function:  void addUppackageCounter()
  * @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
  *------------------------------------------------------------------------------------------------------*/
template<class ClientType>
inline void HCNSTcpServer<ClientType>::addUpPackageCounter()
{
          ++this->m_packageCounter;
}

/*------------------------------------------------------------------------------------------------------
* @function:  void readMessageHeader
* @param:  1.[IN] std::shared_ptr<ClientType> _clientSocket
                    2.[IN]  _PackageHeader* _header

* @description: process clients' message header
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::readMessageHeader(
          IN std::shared_ptr<ClientType> _clientSocket,
          IN  _PackageHeader* _header)
{
          /*add up to HCNSTcpServer package counter*/
          this->addUpPackageCounter();

          std::cout << "Client's Connection Request<Socket =" << static_cast<int>(_clientSocket->getClientSocket()) << ","
                    << inet_ntoa(_clientSocket->getClientAddr()) << ":" << _clientSocket->getClientPort() << "> : "
                    << "Data Length = " << _header->_packageLength << ", Request = ";

          if (_header->_packageCmd == CMD_LOGIN) {
                    std::cout << "CMD_LOGIN";
          }
          else if (_header->_packageCmd == CMD_LOGOUT) {
                    std::cout << "CMD_LOGOUT";
          }
          else if (_header->_packageCmd == CMD_PULSE_DETECTION) {
                    std::cout << "CMD_PULSE_DETECTION";
          }
          else {
          }
}

/*------------------------------------------------------------------------------------------------------
  * @function:  virtual void readMessageBody
  * @param:  1. [IN] HCNSCellServer<ClientType>*: this param is perpare for HCNSCellServer this pointer
                      2. [IN]std::shared_ptr<ClientType> _clientSocket
                      3. [IN] _PackageHeader* _buffer

  * @description:  process clients' message body
  * @problem: in some sceniaro, every cell server thread might generate more newed memory than delete
  *------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::readMessageBody(
          IN HCNSCellServer<ClientType>* _cellServer,
          IN std::shared_ptr<ClientType> _clientSocket,
          IN _PackageHeader* _header
)
{
          _PackageHeader* reply(nullptr);

          /*reset _clientSocket pulse timer*/
          _clientSocket->resetPulseReportedTime();

          if (_header->_packageCmd == CMD_LOGIN) {
                    _LoginData* loginData(reinterpret_cast<_LoginData*>(_header));
                    reply = new _LoginData(loginData->userName, loginData->userPassword);
                    dynamic_cast<_LoginData*>(reply)->loginStatus = true;                  //set login status as true
          }
          else if (_header->_packageCmd == CMD_LOGOUT) {
                    _LogoutData* logoutData(reinterpret_cast<_LogoutData*>(_header));
                    reply = new _LogoutData(logoutData->userName);
                    dynamic_cast<_LogoutData*>(reply)->logoutStatus = true;               //set logout status as true
          }
          else if (_header->_packageCmd = CMD_PULSE_DETECTION)
          {
                    reply = new _PULSE;
          }
          else {
                    reply = new _PackageHeader(sizeof(_PackageHeader), CMD_ERROR);
          }
          _cellServer->pushMessageSendingTask(_clientSocket, reply);
}