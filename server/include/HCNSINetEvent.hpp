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
};

#endif