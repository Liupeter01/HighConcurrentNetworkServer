#pragma once
#ifndef _HCNSTCPSERVER_H_
#define _HCNSTCPSERVER_H_
#include<HCNSCellServer.hpp>

template<class ClientType = _ClientSocket>
class HCNSTcpServer :public INetEvent<ClientType>
{
public:
          HCNSTcpServer();
          HCNSTcpServer(IN unsigned short _ipPort);
          HCNSTcpServer(
                    IN unsigned long _ipAddr, 
                    IN unsigned short _ipPort
          );

          virtual ~HCNSTcpServer();

public:
          static SOCKET createServerSocket(
                    IN int af = AF_INET,
                    IN int type = SOCK_STREAM,
                    IN int protocol = IPPROTO_TCP
          );

          static int startListeningConnection(
                    IN SOCKET serverSocket,
                    IN int backlog = SOMAXCONN
          );

          static bool acceptClientConnection(
                    IN SOCKET serverSocket,
                    OUT SOCKET* clientSocket,
                    OUT SOCKADDR_IN* _clientAddr
          );

public:
          int startServerListening(IN int backlog = SOMAXCONN);
          void serverMainFunction();

private:
          void purgeCloseSocket(ClientType* _pclient);
          void initServerAddressBinding(
                    unsigned long _ipAddr,
                    unsigned short _port
          );

          bool initServerSelectModel(IN SOCKET _largestSocket);
          void serverInterfaceLayer(IN OUT std::promise<bool>& interfacePromise);

          void pushClientToCellServer(IN ClientType* _client);
          void clientConnectionThread();

          void getClientsUploadSpeed();

          void shutdownTcpServer();

private:
          virtual void clientOnLeave(ClientType* _pclient);
          virtual void addUpClientsCounter();
          virtual void decreaseClientsCounter();
          virtual void addUpPackageCounter();

private:
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

          /*HCNSCellServer(Consumer Thread)*/
          std::vector<HCNSCellServer<ClientType>*> m_cellServer;

          /*HCNSCellServer should record all the connection*/
          std::mutex m_clientRWLock;
          typename std::vector<ClientType*> m_clientInfo;

          /*record packages received in HCNSCellServer container*/
          std::atomic<unsigned long long> m_packageCounter;

          /*record clients connected to HCNSCellServer container*/
          std::atomic<unsigned long long> m_ClientsCounter;

          /*Server High Resolution Clock Model(How Many Packages are recieved per second)*/
          HCNSTimeStamp* m_timeStamp;

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
HCNSTcpServer<ClientType>::HCNSTcpServer(IN unsigned short _ipPort)
          :HCNSTcpServer(INADDR_ANY, _ipPort)
{
}

template<class ClientType>
HCNSTcpServer<ClientType>::HCNSTcpServer(
          IN unsigned long _ipAddr, 
          IN unsigned short _ipPort)
          : m_timeStamp(new HCNSTimeStamp()),
          m_interfaceFuture(this->m_interfacePromise.get_future()),
          m_packageCounter(0)
{
#if _WIN32                          //Windows Enviorment
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif
          this->initServerAddressBinding(_ipAddr, _ipPort);
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
* use socket api to create a ipv4 and tcp protocol socket
* @function: static SOCKET createServerSocket
* @param :  1.[IN] SOCKET  serverSocket
*                   2.[IN] int backlog
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
int HCNSTcpServer<ClientType>::startListeningConnection(IN SOCKET serverSocket, IN int backlog)
{
          return ::listen(serverSocket, backlog);
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
          std::cout << "Accept Client's Connection<Socket =" << static_cast<int>(*clientSocket) << ","
                    << inet_ntoa(_clientAddr->sin_addr) << ":" << _clientAddr->sin_port << ">" << std::endl;

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
          return startListeningConnection(this->m_server_socket, backlog);
}

/*------------------------------------------------------------------------------------------------------
* start to create server thread
* @function:  void serverMainFunction
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::serverMainFunction()
{
          th_tcpServerThreadPool.emplace_back(
                    std::mem_fn(&HCNSTcpServer::clientConnectionThread), this
          );
          th_tcpServerThreadPool.emplace_back(
                    std::mem_fn(&HCNSTcpServer::serverInterfaceLayer), this, std::ref(this->m_interfacePromise)
          );

          for (int i = 0; i < 4; ++i) {
                    this->m_cellServer.push_back(
                              new HCNSCellServer<ClientType>(
                                        this->m_server_socket,
                                        this->m_server_address,
                                        this->m_interfaceFuture,
                                        this
                              )
                    );
          }
          for (auto ib = this->m_cellServer.begin(); ib != this->m_cellServer.end(); ib++) {
                    (*ib)->startCellServer();
          }
}

/*------------------------------------------------------------------------------------------------------
* shutdown and terminate network connection
* @function: void pushTemproaryClient(ClientType* _pclient)
* @retvalue: ClientType* _pclient
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::purgeCloseSocket(ClientType* _pclient)
{
          this->decreaseClientsCounter();
#if _WIN32
          ::shutdown(_pclient->getClientSocket(), SD_BOTH);                        //disconnect I/O
          ::closesocket(_pclient->getClientSocket());                                        //release socket completely!! 
#else 
          ::shutdown(_pclient->getClientSocket(), SHUT_RDWR);                 //disconnect I/O and keep recv buffer
          ::close(_pclient->getClientSocket());                                                   //release socket completely!! 
#endif
          delete _pclient;
}

/*------------------------------------------------------------------------------------------------------
* init server ip address and port and bind it with server socket
* @function: void initServerAddressBinding
* @implementation: create server socket and bind ip address info
* @param : 1.[IN] unsigned long _ipAddr
                    2.[IN] unsigned short _port
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::initServerAddressBinding(
          unsigned long _ipAddr,
          unsigned short _port)
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
void HCNSTcpServer<ClientType>::serverInterfaceLayer(
          IN OUT std::promise<bool>& interfacePromise)
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
* push client's structure into a std::vector<HCNSCellServer*>::iterator
* the total ammount of the clients should be the lowest among other  std::vector<HCNSCellServer*>::iterator
* @function: void :pushClientToCellServer(ClientType* _client)
* @param: ClientType* _client
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::pushClientToCellServer(IN ClientType* _client)
{
          /* add up to m_clientCounter */
          this->addUpClientsCounter();                               
          this->m_clientInfo.push_back(_client);

          auto _lowest = this->m_cellServer.begin();
          for (auto ib = this->m_cellServer.begin(); ib != this->m_cellServer.end(); ++ib) {

                    /*find out which cell server has the lowest client handling load*/
                    if ((*_lowest)->getClientsConnectionLoad() > (*ib)->getClientsConnectionLoad()) {
                              _lowest = ib;
                    }
          }
          (*_lowest)->pushTemproaryClient(_client);
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
                    FD_ZERO(&this->m_fdread);                                                               //clean fd_read
                    FD_SET(this->m_server_socket, &this->m_fdread);                           //Insert Server Socket into fd_read
                   
                    /*the number of server socket is the largest in client connection thread(producer)*/
                    if (this->initServerSelectModel(this->m_server_socket)) {
                              this->shutdownTcpServer();                 //when select model fails then shutdown server
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
                              this->pushClientToCellServer(new ClientType(_clientSocket, _clientAddress));
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
                              << "<Connections:" << this->m_ClientsCounter << "> " << "Client's Upload Speed = "
                              << this->m_packageCounter / this->m_timeStamp->getElaspsedTimeInsecond()
                              << " Packages/s" << std::endl;

                    /*reset package counter*/
                    this->m_packageCounter = 0;

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
                    this->purgeCloseSocket((*ib));
          }
          this->m_clientInfo.clear();

          /*close all the clients' connection in this cell server*/
          for (auto ib = this->m_cellServer.begin(); ib != this->m_cellServer.end(); ib++){
                    delete (*ib);
          }
          this->m_cellServer.clear(); 

          for (auto ib = this->th_tcpServerThreadPool.begin(); ib != this->th_tcpServerThreadPool.end(); ib++) {
                    if (ib->joinable()) {
                              ib->join();
                    }
          }

#if _WIN32             
          ::shutdown(this->m_server_socket, SD_BOTH);                     //disconnect I/O
          ::closesocket(this->m_server_socket);                                      //release socket completely!! 
#else                                   
          ::shutdown(this->m_server_socket, SHUT_RDWR);               //disconnect I/O and keep recv buffer
          close(this->m_server_socket);                                                   //release socket completely!! 
#endif

          /*reset socket*/
          this->m_server_socket = INVALID_SOCKET;

          /*delete server high resolution clock model*/
          delete this->m_timeStamp;

          /*cleanup wsa setup*/
#if _WIN32                          
          WSACleanup();
#endif
}

/*------------------------------------------------------------------------------------------------------
* virtual function: client terminate connection
* @function:  void clientOnLeave(ClientType* _pclient)
* @param :  ClientType* _pclient
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::clientOnLeave(ClientType* _pclient)
{
          /* decrease m_clientCounter */
          this->decreaseClientsCounter();

          /*
          * in clientOnLeave function, the scale of the lock have to cover the all block of the code
          * in this sceniro we have to use std::vector::size() function instead of using the increment of std::vector<ClientType*>::iterator
          */
          std::lock_guard<std::mutex> _lckg(this->m_clientRWLock);
          for (int i = 0; i < this->m_clientInfo.size(); ++i) {
                    if (this->m_clientInfo[i] == _pclient) {
                              typename std::vector<ClientType*>::iterator iter = this->m_clientInfo.begin() + i;
                              if (iter != this->m_clientInfo.end()) {
                                        this->m_clientInfo.erase(iter);
                              }
                    }
          }
}

/*------------------------------------------------------------------------------------------------------
  * virtual function: decrease to the number of clients
  * @function:  decreaseClientsCounter()
  *------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::decreaseClientsCounter()
{
          --this->m_ClientsCounter;
}

/*------------------------------------------------------------------------------------------------------
    * virtual function: add up to the number of clients
    * @function:  void addUpClientsCounter()
    *------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::addUpClientsCounter()
{
          ++this->m_ClientsCounter;
}

/*------------------------------------------------------------------------------------------------------
  * virtual function: add up to the number of packages being received
  * @function:  void addUpPackageCounter()
  *------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSTcpServer<ClientType>::addUpPackageCounter()
{
          ++this->m_packageCounter;
}