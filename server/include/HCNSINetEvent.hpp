#pragma once
#ifndef _HCNSINETEVENT_H_
#define _HCNSINETEVENT_H_
#include<ClientSocket.h>

template<typename ClientType = _ClientSocket>
class INetEvent
{
public:
		  INetEvent() {}
		  virtual ~INetEvent() {}
public:
		  /*------------------------------------------------------------------------------------------------------
		  * virtual function: client terminate connection
		  * @function:  void clientOnLeave(ClientType * _pclient) 
		  * @param : ClientType * _pclient
		  *------------------------------------------------------------------------------------------------------*/
		  virtual void clientOnLeave(ClientType * _pclient) = 0;

		  /*------------------------------------------------------------------------------------------------------
			* virtual function: decrease to the number of clients
			* @function:  decreaseClientsCounter()
			*------------------------------------------------------------------------------------------------------*/
		  virtual void decreaseClientsCounter() = 0;

		  /*------------------------------------------------------------------------------------------------------
			  * virtual function: add up to the number of clients
			  * @function:  void addUpClientsCounter()
			  *------------------------------------------------------------------------------------------------------*/
		  virtual void addUpClientsCounter() = 0;

		  /*------------------------------------------------------------------------------------------------------
			* virtual function: add up to the number of packages being received
			* @function:  void addUppackageCounter()
			*------------------------------------------------------------------------------------------------------*/
		  virtual void addUpPackageCounter() = 0;
};

#endif