#pragma once
#ifndef _CONNECTIONCONTROLLER_H_
#define _CONNECTIONCONTROLLER_H_
#include<CellClient.hpp>

class ConnectionController :public INetEvent
{
public:
		  typedef  std::function<void(std::shared_ptr<_ServerSocket>, char*, int)> CellClientTask;
		  typedef std::pair<std::shared_ptr<_ServerSocket>, CellClientTask> CellClientPackage;

		  typedef std::vector<CellClientPackage> value_type;
		  typedef typename value_type::iterator iterator;

public:
		  ConnectionController(IN long long _timeout);
		  ConnectionController(IN size_t threadNumber, IN long long _timeout);
		  virtual ~ConnectionController();

public:
		  void addServerConnection(
					IN unsigned long _ipAddr,
					IN unsigned short _ipPort,
					IN CellClientTask&& _dosth =
					[&](std::shared_ptr<_ServerSocket> _serverSocket, char* _szSendBuf, int _szBufferSize)->void {
							  _serverSocket->sendDataToServer(_szSendBuf, _szBufferSize);
					}
		  );
		  void connCtrlMainFunction(unsigned int _threadNumber = 1);

private:
		  static SOCKET createSocket(
					IN int af = AF_INET,
					IN int type = SOCK_STREAM,
					IN int protocol = IPPROTO_TCP
		  );
		  
private:
		  std::vector<std::shared_ptr<CellClient>>::iterator  findLowestConnectionLoad();
		  void userOperateInterface(IN OUT std::promise<bool>& interfacePromise);

		  void purgeCloseSocket(IN typename  iterator _pserver);
		  void shutdownAllConnection();

private:
		  virtual void serverComTerminate(IN typename INetEvent::iterator _pserver);
		  virtual inline void readMessageHeader(
					IN  std::shared_ptr <_ServerSocket> _clientSocket,
					IN _PackageHeader* _header
		  );
		  virtual inline void readMessageBody(
					IN CellClient* _cellServer,
					IN std::shared_ptr <_ServerSocket> _clientSocket,
					IN _PackageHeader* _header
		  );

private:
		  /*Client and Thread Number Setting*/
		  std::atomic< std::size_t> _clientNumber = 0;
		  std::atomic< std::size_t>_threadNumber = 1;

		  /*Client Pulse Timeout Setting*/
		  long long _reportTimeSetting;

		  /*server interface symphoare control and thread creation*/
		  std::promise<bool> m_interfacePromise;
		  std::shared_future<bool> m_interfaceFuture;

		  /*threadpool*/
		  std::vector<std::thread> th_ThreadPool;

		  /*
		  * std::shared_ptr<CellClient> (Consumer Thread)
		  * add memory smart pointer to control memory allocation
		  */
		  std::vector<std::shared_ptr<CellClient>> m_cellClient;

		  /*
		  * std::shared_ptr<_ServerSocket>
		  * CellClient should record all the connection
		  * add memory smart pointer to record all the connection
		  */
		  std::mutex m_serverRWLock;
		  value_type m_serverInfo;

#if _WIN32 
		  WSADATA m_wsadata;
#endif // _WINDOWS 

};

#endif // !_CONNECTIONCONTROLLER_H_
