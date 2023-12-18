#include<CellClient.hpp>

CellClient::CellClient(
          IN std::shared_future<bool>& _future, 
          IN INetEvent* _netEvent, 
          IN long long timeout)
          :m_interfaceFuture(_future),
          m_pNetEvent(_netEvent),
          _reportTimeSetting(timeout),
          m_isServerArrayChanged(true)
{
}

CellClient::~CellClient()
{
          this->shutdownCellClient();
}

/*------------------------------------------------------------------------------------------------------
* start cell client and create a thread for serverMsgProcessingThread
* @function: void startCellClient()
*------------------------------------------------------------------------------------------------------*/
void CellClient::startCellClient()
{
          this->th_serverMsgProcessing = std::thread(
                    std::mem_fn(&CellClient::serverMsgProcessingThread), this, std::ref(this->m_interfaceFuture)
          );

          /*excute std::function which is stored in this->m_ServerPoolVec */
          this->th_serverMsgSending = std::thread(
                    std::mem_fn(&CellClient::serverMsgSendingThread), this, std::ref(this->m_interfaceFuture)
          );
}

/*------------------------------------------------------------------------------------------------------
* how many server are established inside the  container(including those inside the permanent and temproary queue)
* @function: size_t getConnectionLoad
* @retvalue: size_t
*------------------------------------------------------------------------------------------------------*/
size_t CellClient::getConnectionLoad()
{
          return this->m_temporaryServerPoolBuffer.size() + this->m_ServerPoolVec.size();
}

/*------------------------------------------------------------------------------------------------------
* expose an interface for consumer to insert client connection into the temproary container
* @function: void pushTemproary(IN std::shared_ptr<_ServerSocket> _pclient,IN std::function<void()>&& _dosth)
* @param: 1.[IN] std::shared_ptr<ClientType> _pclient
*                 2.[IN]  CellClientTask&& _dosth
* 
* @warning!: PLEASE DO NOT USE RIGHT VALUE(std::move)!!! IT IS A COPY!!!
*------------------------------------------------------------------------------------------------------*/
void CellClient::pushTemproary(IN std::shared_ptr<_ServerSocket> _pclient, IN  CellClientTask&& _dosth)
{
          std::lock_guard<std::mutex> _lckg(this->m_queueMutex);
          this->m_temporaryServerPoolBuffer.emplace_back(
                    _pclient,
                    std::forward<decltype(_dosth)>(_dosth)
          );
}

/*------------------------------------------------------------------------------------------------------
* get largest socket value
* @function: int getLargestSocketValue()
* @retvalue: int
*------------------------------------------------------------------------------------------------------*/
int CellClient::getLargestSocketValue()
{
          return static_cast<int>(this->m_largestSocket) + 1;
}

/*------------------------------------------------------------------------------------------------------
* init IO Multiplexing
* @function: void initClientIOMultiplexing
* @description: in client, we only need to deal with client socket
*------------------------------------------------------------------------------------------------------*/
void CellClient::initClientIOMultiplexing()
{
          /*is server connection array changed*/
          if (this->m_isServerArrayChanged) {
                    FD_ZERO(&m_fdread);                                                              //clean fd_read

                    this->m_isServerArrayChanged = false;
                    this->m_largestSocket = static_cast<SOCKET>(0);

                    /*add all the client socket in to the fd_read*/
                    std::lock_guard<std::mutex> _lckg(this->m_queueMutex);
                    std::for_each(this->m_ServerPoolVec.begin(), this->m_ServerPoolVec.end(),
                              [&](std::pair<std::shared_ptr<_ServerSocket>, CellClientTask>& _server)
                              {
                                        if (_server.first != nullptr && _server.second != nullptr) {
                                                  FD_SET(_server.first->getServerSocket(), &m_fdread);
                                                  if (m_largestSocket < _server.first->getServerSocket()) {
                                                            m_largestSocket = _server.first->getServerSocket();
                                                  }
                                        }
                              }
                    );
#if _WIN32    
                    memcpy_s(
                              reinterpret_cast<void*>(&this->m_fdreadCache),
                              sizeof(fd_set),
                              reinterpret_cast<void*>(&this->m_fdread),
                              sizeof(fd_set)
                    );
#else             
                    memcpy(
                              reinterpret_cast<void*>(&this->m_fdreadCache),
                              reinterpret_cast<void*>(&this->m_fdread),
                              sizeof(fd_set)
                    );
#endif
          }
          else
          {
#if _WIN32    
                    memcpy_s(
                              reinterpret_cast<void*>(&this->m_fdread),
                              sizeof(fd_set),
                              reinterpret_cast<void*>(&this->m_fdreadCache),
                              sizeof(fd_set)
                    );

#else             
                    memcpy(
                              reinterpret_cast<void*>(&this->m_fdread),
                              reinterpret_cast<void*>(&this->m_fdreadCache),
                              sizeof(fd_set)
                    );
#endif
          }
}

/*------------------------------------------------------------------------------------------------------
* use select system call to setup model
* @function: int initClientSelectModel()
* @retvalue: int
*------------------------------------------------------------------------------------------------------*/
int CellClient::initClientSelectModel()
{
          return ::select(
                    getLargestSocketValue(),
                    &m_fdread,
                    nullptr,
                    nullptr,
                    &this->m_timeoutSetting
          );
}

/*------------------------------------------------------------------------------------------------------
* shutdown and terminate network connection
* @function:purgeCloseSocket(IN iterator_pserver)
* @param: [IN] iterator
*------------------------------------------------------------------------------------------------------*/
void CellClient::purgeCloseSocket(IN iterator _pserver)
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
* @function:  serverDataProcessingLayer
* @param: [IN] iterator  _serverSocket
* @description: process the data transfer from server
* @retvalue : bool
* @update: in order to enhance the performance of the server, we are going to
*           remove m_szRecvBuffer in cellserver and serveral memcpy functions
*------------------------------------------------------------------------------------------------------*/
bool  CellClient::serverDataProcessingLayer(IN iterator _serverSocket)
{
          _PackageHeader* _header(reinterpret_cast<_PackageHeader*>(
                    _serverSocket->first->getMsgBufferHead()
                    ));
          
          /*clean the recv buffer*/
          memset(_serverSocket->first->getMsgBufferHead(), 0, _serverSocket->first->getBufFullSpace());

          /* recv and store data in every _serverSocket */
          int  recvStatus = _serverSocket->first->reciveDataFromServer(           //retrieve data from kernel buffer space
                    _serverSocket->first->getMsgBufferHead(),
                    _serverSocket->first->getBufRemainSpace()
          );

          if (recvStatus <= 0) {                                             //no data recieved!
                    std::cout << "Server's Connection Terminate<Socket =" << _serverSocket->first->getServerSocket() << ","
                              << inet_ntoa(_serverSocket->first->getServerAddr()) << ":"
                              << _serverSocket->first->getServerPort() << ">" << std::endl;

                    return false;
          }
          else {
                    _serverSocket->first->increaseMsgBufferPos(recvStatus);         //update the pointer position 

                    /* judge whether the length of the data in message buffer is bigger than the sizeof(_PackageHeader) */
                    while (_serverSocket->first->getMsgPtrPos() >= sizeof(_PackageHeader)) {

                              /*the size of current message in szMsgBuffer is bigger than the package length(_header->_packageLength)*/
                              if (_header->_packageLength <= _serverSocket->first->getMsgPtrPos()) {

                                        //get message header to indentify commands
                                        //this->m_pNetEvent->readMessageHeader(_serverSocket->first, reinterpret_cast<_PackageHeader*>(_header));
                                        //this->m_pNetEvent->readMessageBody(this, _serverSocket->first, reinterpret_cast<_PackageHeader*>(_header));

                                        /*
                                         * delete this message package and modify the array
                                         */
#if _WIN32     //Windows Enviorment
                                        memcpy_s(
                                                  _serverSocket->first->getMsgBufferHead(),                                                     //the head of message buffer array
                                                  _serverSocket->first->getBufFullSpace(),
                                                  _serverSocket->first->getMsgBufferHead() + _header->_packageLength,       //the next serveral potential packages position
                                                  _serverSocket->first->getMsgPtrPos() - _header->_packageLength                  //the size of next serveral potential package
                                        );

#else               /* Unix/Linux/Macos Enviorment*/
                                        memcpy(
                                                  _serverSocket->first->getMsgBufferHead(),                                                      //the head of message buffer array
                                                  _serverSocket->first->getMsgBufferHead() + _header->_packageLength,       //the next serveral potential packages position
                                                  _serverSocket->first->getMsgPtrPos() - _header->_packageLength                  //the size of next serveral potential package
                                        );
#endif
                                        _serverSocket->first->decreaseMsgBufferPos(_header->_packageLength);       //update the pointer pos of message array                       //recalculate the size of the rest of the array
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
          }

          /*clean the recv buffer*/
          memset(_serverSocket->first->getMsgBufferHead(), 0, _serverSocket->first->getBufFullSpace());
          return true;
}

/*------------------------------------------------------------------------------------------------------
* processing server msg(Consumer Thread)
* @function: void serverMsgProcessingThread(IN std::shared_future<bool>& _future)
* @param: IN std::shared_future<bool>& _future
*------------------------------------------------------------------------------------------------------*/
void CellClient::serverMsgSendingThread(IN std::shared_future<bool>& _future)
{
          while (true)
          {
                    if (_future.wait_for(std::chrono::microseconds(1)) == std::future_status::ready) {
                              if (!_future.get()) {
                                        break;
                              }
                    }
                    
                    /*if there is nothing in container then continue!*/
                    if (!this->m_ServerPoolVec.size()) {
                              std::this_thread::sleep_for(std::chrono::milliseconds(1));
                              continue;
                    }

                    std::lock_guard<std::mutex> _lckg(this->m_queueMutex);
                    for (auto ib = this->m_ServerPoolVec.begin(); ib != this->m_ServerPoolVec.end(); ++ib) {
                              ib->second(ib->first, reinterpret_cast<char*>(&loginData), sizeof(loginData));
                    }
          }
}

/*------------------------------------------------------------------------------------------------------
* processing server msg(Consumer Thread)
* @function: void serverMsgProcessingThread(IN std::shared_future<bool>& _future)
* @param: IN std::shared_future<bool>& _future
*------------------------------------------------------------------------------------------------------*/
void CellClient::serverMsgProcessingThread(IN std::shared_future<bool>& _future)
{
          while (true)
          {
                    if (_future.wait_for(std::chrono::microseconds(1)) == std::future_status::ready) {
                              if (!_future.get()) {
                                        break;
                              }
                    }

                    /*size of temporary buffer is valid*/
                    if (this->m_temporaryServerPoolBuffer.size())
                    {
                              std::lock_guard<std::mutex> _lckg(this->m_queueMutex);
                              std::for_each(this->m_temporaryServerPoolBuffer.begin(), this->m_temporaryServerPoolBuffer.end(),
                                        [&](std::pair<std::shared_ptr<_ServerSocket>, CellClientTask>& _server){
                                                  this->m_ServerPoolVec.push_back(_server);
                                        }
                              );
                              this->m_temporaryServerPoolBuffer.clear();

                              /*new server connection established*/
                              this->m_isServerArrayChanged = true;
                    }

                    /*
                    * size of permanent container should be valid
                    * suspend this thread for 1 millisecond in order to block this thread from occupying cpu cycle
                    */
                    if (this->m_ServerPoolVec.size() <= 0) {
                              std::this_thread::sleep_for(std::chrono::milliseconds(1));
                              continue;
                    }

                    this->initClientIOMultiplexing();

                    int _selectStatus = this->initClientSelectModel();

                    /*select model mulfunction*/
                    if (_selectStatus < 0) {
                              this->shutdownCellClient();
                              break;
                    }
                    else if (!_selectStatus) {         /*cellserver hasn't receive any data*/
                              continue;
                    }

                    for (auto ib = this->m_ServerPoolVec.begin(); ib != this->m_ServerPoolVec.end();)
                    {

                              /*Detect client message input signal*/
                              if (FD_ISSET(ib->first->getServerSocket(), &m_fdread))
                              {

                                        /*
                                        *Entering main logic layer std::vector<_ServerSocket>::iterator as an input to the main system
                                        * retvalue: when functionlogicLayer return a value of false it means [SERVER TERMINATE]
                                        * then you have to remove it from the container
                                        */
                                        if (!this->serverDataProcessingLayer(ib))
                                        {
                                                  /*server connection terminate leave*/
                                                  this->m_isServerArrayChanged= true;

                                                  {
                                                            std::lock_guard<std::mutex> _lckg(this->m_queueMutex);

                                                            /*notify the connectioncontroller to delete it from the container and dealloc it's memory*/
                                                            if (this->m_pNetEvent != nullptr) {
                                                                      this->m_pNetEvent->serverComTerminate(ib);
                                                            }

                                                            /*modify code from just delete (*ib) to shutdown socket first and then delete*/
                                                            this->purgeCloseSocket(ib);

                                                            /* erase Current unavailable client's socket(no longer needs to dealloc it's memory)*/
                                                            ib = this->m_ServerPoolVec.erase(ib);
                                                  }

                                                  /*
                                                    * There is a kind of sceniro when delete socket obj is completed
                                                    * and there is only one client still remainning connection to the server
                                                    */
                                                  if (ib == this->m_ServerPoolVec.end() || this->m_ServerPoolVec.size() <= 1) {
                                                            break;
                                                  }
                                        }
                              }
                              ib++;
                    }
          }
}

/*------------------------------------------------------------------------------------------------------
* close and erase all the connection and cleanup network setup
* @function: void shutdownCellClient()
*------------------------------------------------------------------------------------------------------*/
void  CellClient::shutdownCellClient()
{
          /*-------------------------------------------------------------------------
          * close all the  connection in this cell client
          *-------------------------------------------------------------------------*/
          for (auto ib = this->m_ServerPoolVec.begin(); ib != this->m_ServerPoolVec.end(); ib++)
          {
                    if (this->m_pNetEvent != nullptr) {
                              this->m_pNetEvent->serverComTerminate(ib);
                    }
                    this->purgeCloseSocket(ib);
          }
          this->m_ServerPoolVec.clear();                                            //clean the std::vector container

          /*processing all the thread*/
          if (this->th_serverMsgProcessing.joinable()) {
                    this->th_serverMsgProcessing.join();
          }
          if (this->th_serverMsgSending.joinable()) {
                    this->th_serverMsgSending.join();
          }
}