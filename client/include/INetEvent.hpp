#pragma once
#ifndef _INETEVENT_H_
#define _INETEVENT_H_
#include<vector>
#include<functional>
#include<algorithm>

#include<DataPackage.h>
#include<ServerSocket.hpp>

class CellClient;

class INetEvent
{
public:
		  typedef  std::function<void(std::shared_ptr<_ServerSocket>, char*, int)> CellClientTask;
		  typedef std::pair<std::shared_ptr<_ServerSocket>, CellClientTask> CellClientPackage;

		  typedef std::vector<CellClientPackage> value_type;
		  typedef typename value_type::iterator iterator;

public:
		  INetEvent() {}
		  virtual ~INetEvent() {}

public:
		  /*------------------------------------------------------------------------------------------------------
			* virtual function: server communication terminate
			* @function:  serverComTerminate(IN iterator _pserver)
			* @param : [IN] iterator _pserver
			* @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
			*------------------------------------------------------------------------------------------------------*/
		  virtual void serverComTerminate(IN iterator _pserver) = 0;

		  /*------------------------------------------------------------------------------------------------------
			* @function:  void readMessageHeader
			* @param:  1.[IN] std::shared_ptr <_ServerSocket> _clientSocket
								2.[IN ] _PackageHeader* _header

			* @description: process  message header
			*------------------------------------------------------------------------------------------------------*/
		  virtual inline void readMessageHeader(
					IN  std::shared_ptr <_ServerSocket> _clientSocket,
					IN _PackageHeader* _header
		  ) = 0;

		  /*------------------------------------------------------------------------------------------------------
			* @function:  virtual void readMessageBody
			* @param:  	1. [IN] CellClient* _cellServer,
								2. [IN] std::shared_ptr <_ServerSocket> _clientSocket
								3. [IN] _PackageHeader* _header

			* @description:  process  message body
			*------------------------------------------------------------------------------------------------------*/
		  virtual inline void readMessageBody(
					IN CellClient* _cellServer,
					IN std::shared_ptr <_ServerSocket> _clientSocket,
					IN _PackageHeader* _header
		  ) = 0;
};

#endif