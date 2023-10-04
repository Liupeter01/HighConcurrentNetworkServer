#pragma once
#ifndef _HCNSINETEVENT_H_
#define _HCNSINETEVENT_H_
#include<vector>
#include<DataPackage.h>
#include<ClientSocket.hpp>

template<typename ClientType = _ClientSocket>
class INetEvent
{
public:
		  INetEvent() {}
		  virtual ~INetEvent() {}
public:
		  /*------------------------------------------------------------------------------------------------------
			* virtual function: client connect to server
			* @function:  void clientOnJoin(ClientType * _pclient)
			* @param : ClientType * _pclient
			* @multithread safety issue: will only be triggered by only one thread
			*------------------------------------------------------------------------------------------------------*/
		  virtual inline void clientOnJoin(ClientType* _pclient) = 0;

		  /*------------------------------------------------------------------------------------------------------
		  * virtual function: client terminate connection
		  * @function:  void clientOnLeave(ClientType * _pclient) 
		  * @param : ClientType * _pclient
		  * @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
		  *------------------------------------------------------------------------------------------------------*/
		  virtual void clientOnLeave(ClientType * _pclient) = 0;

		  /*------------------------------------------------------------------------------------------------------
		   * virtual function: add up to the number of recv function calls
		   * @function:  void addUpRecvCounter(ClientType* _pclient)
		   * @param : ClientType * _pclient
		   * @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
		   *------------------------------------------------------------------------------------------------------*/
		  virtual inline void addUpRecvCounter(ClientType* _pclient) = 0;

		  /*------------------------------------------------------------------------------------------------------
			* virtual function: add up to the number of clients
			* @function:  void addUpClientsCounter()
			* @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
			*------------------------------------------------------------------------------------------------------*/
		  virtual inline void addUpClientsCounter() = 0;

		  /*------------------------------------------------------------------------------------------------------
			* virtual function: decrease to the number of clients
			* @function:  decreaseClientsCounter()
			* @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
			*------------------------------------------------------------------------------------------------------*/
		  virtual inline void decreaseClientsCounter() = 0;

		  /*------------------------------------------------------------------------------------------------------
			* virtual function: add up to the number of packages being received
			* @function:  void addUppackageCounter()
			* @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
			*------------------------------------------------------------------------------------------------------*/
		  virtual inline void addUpPackageCounter() = 0;

		  /*------------------------------------------------------------------------------------------------------
		  * @function:  void readMessageHeader
		  * @param:  1.[IN] typename std::vector<ClientType*>::iterator _clientSocket
							  2.[IN ] _PackageHeader* _header

		  * @description: process clients' message header
		  *------------------------------------------------------------------------------------------------------*/
		  virtual inline void readMessageHeader(
					IN  typename  std::vector<ClientType*>::iterator _clientSocket,
					IN _PackageHeader* _header
		  ) = 0;

		  /*------------------------------------------------------------------------------------------------------
		  * @function:  virtual void readMessageBody
		  * @param:  1.[IN] typename std::vector<ClientType*>::iterator _clientSocket
							  2.[IN] _PackageHeader* _buffer

		  * @description:  process clients' message body
		  *------------------------------------------------------------------------------------------------------*/
		  virtual inline void readMessageBody(
					IN typename  std::vector<ClientType*>::iterator _clientSocket,
					IN _PackageHeader* _header
		  ) = 0;
};

#endif