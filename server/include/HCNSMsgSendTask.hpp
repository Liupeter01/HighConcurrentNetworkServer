#pragma once
#ifndef _HCNSMSGSENDTASK_H_
#define _HCNSMSGSENDTASK_H_
#include<DataPackage.h>
#include<ClientSocket.hpp>
#include<HCNSTaskDispatcher.h>
#include<HCNSMemoryManagement.hpp>

#if _WIN32
#pragma comment(lib,"HCNSMemoryPool.lib")
#endif

template<typename T> T memory_alloc(size_t _size)
{
          return reinterpret_cast<T>(::malloc(_size));
}

template<typename T> void memory_free(T _ptr)
{
          ::free(reinterpret_cast<void*>(_ptr));
}

/*transmit send buffer data to the clients*/
template<class ClientType = _ClientSocket>
class HCNSSendTask :public HCNSCellTask
{
public:
          HCNSSendTask(ClientType* _client, _PackageHeader* _header)
                    :m_pClient(_client), m_packageHeader(_header) 
          {}
          virtual ~HCNSSendTask() 
          {
                    delete m_pClient;
                    delete m_packageHeader;
          }

public:
          virtual void excuteTask();

private:
          ClientType* m_pClient;
          _PackageHeader* m_packageHeader;
};
#endif

/*------------------------------------------------------------------------------------------------------
* send data package(m_packageHeader) to the client(m_pClient) async
* @function: void excuteTask
*------------------------------------------------------------------------------------------------------*/
template<class ClientType>
void HCNSSendTask<ClientType>::excuteTask()
{
          this->m_pClient->sendDataToClient(
                    this->m_packageHeader, 
                    this->m_packageHeader->_packageLength
          );

          /*we allocate memory at HCNSMsgSendTask::pushMessageSendingTask*/
          delete this->m_packageHeader;
}