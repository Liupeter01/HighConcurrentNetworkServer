#pragma once
#ifndef _HCNSMSGSENDTASK_H_
#define _HCNSMSGSENDTASK_H_
#include<DataPackage.h>
#include<ClientSocket.hpp>
#include<HCNSTaskDispatcher.h>
#include<HCNSObjectPool.hpp>

#if _WIN32
#pragma comment(lib,"HCNSMemoryObjectPool.lib")
#endif

/*transmit send buffer data to the clients*/
template<class ClientType = _ClientSocket>
class HCNSSendTask :public HCNSCellTask
{
public:
          HCNSSendTask() = default;
          HCNSSendTask(std::shared_ptr<ClientType> _client, std::shared_ptr<_PackageHeader>&& _header)
                    :m_pClient(_client), m_packageHeader(_header) 
          {}
          virtual ~HCNSSendTask() = default;

public:
          virtual void excuteTask();

private:
          std::shared_ptr<ClientType> m_pClient;
          std::shared_ptr<_PackageHeader> m_packageHeader;
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
                    this->m_packageHeader.get(), 
                    this->m_packageHeader->_packageLength
          );
}