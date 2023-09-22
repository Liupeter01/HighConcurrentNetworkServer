#pragma once
#ifndef _HCNSINETEVENT_H_
#define _HCNSINETEVENT_H_
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
		  virtual void clientOnJoin(ClientType* _pclient) = 0;

		  /*------------------------------------------------------------------------------------------------------
		  * virtual function: client terminate connection
		  * @function:  void clientOnLeave(ClientType * _pclient) 
		  * @param : ClientType * _pclient
		  * @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
		  *------------------------------------------------------------------------------------------------------*/
		  virtual void clientOnLeave(ClientType * _pclient) = 0;

		  /*------------------------------------------------------------------------------------------------------
			* virtual function: decrease to the number of clients
			* @function:  decreaseClientsCounter()
			* @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
			*------------------------------------------------------------------------------------------------------*/
		  virtual void decreaseClientsCounter() = 0;

		  /*------------------------------------------------------------------------------------------------------
		  * virtual function: add up to the number of clients
			* @function:  void addUpClientsCounter()
			* @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
			*------------------------------------------------------------------------------------------------------*/
		  virtual void addUpClientsCounter() = 0;

		  /*------------------------------------------------------------------------------------------------------
			* virtual function: add up to the number of packages being received
			* @function:  void addUppackageCounter()
			* @multithread safety issue: will be triggered by multipule threads, variables should be locked or atomic variables
			*------------------------------------------------------------------------------------------------------*/
		  virtual void addUpPackageCounter() = 0;
};

#endif