#pragma once
#ifndef _HCNSOBJECTPOOLBASE_
#define _HCNSOBJECTPOOLBASE_

#include<iostream>
#include<mutex>
#include<HCNSMemoryManagement.hpp>

#if _WIN32
#pragma comment(lib,"HCNSMemoryPool.lib")
#endif

/*------------------------------------------------------------------------------------------------------
* HCNSNodeDescriptor
* 4B memory alignment
*------------------------------------------------------------------------------------------------------*/
#pragma pack(push)
#pragma pack(4)
struct NodeDescriptor
{
		  NodeDescriptor* pNext;	//next Node pointer
		  uint32_t m_ref;								//reference counter
		  bool m_status;							    //is this node still exist in memory?
};
#pragma pack(pop)

template<typename _Tp, std::size_t _ObjPoolSize>
class HCNSObjectPoolAllocator
{
public:
		  HCNSObjectPoolAllocator();
		  virtual ~HCNSObjectPoolAllocator();
		  _Tp* allocObjectMemory(size_t _size);
		  void deallocObjectMemory(_Tp* _ptr);

private:
		  void initObjectPool();
		  NodeDescriptor* getUnAmbigousHeaderValue();

private:
		  NodeDescriptor* m_pHead;						  //the head of the node
		  char* m_pBuffer;			  //the head of memory
		  bool m_initStatus;										  //objectPool init status
		  std::mutex m_mutex;							      //the mutex
};

#endif
