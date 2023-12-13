#pragma once
#ifndef _HCNSOBJECTPOOLBASE_
#define _HCNSOBJECTPOOLBASE_

#include<HCNSMemoryAllocator.hpp>

/*
 *detour global memory allocation and deallocation functions
 */
void* operator new(size_t _size);
void operator delete(void* _ptr);
void* operator new[](size_t _size);
void operator delete[](void* _ptr);

/*------------------------------------------------------------------------------------------------------
* HCNSNodeDescriptor
* 4B memory alignment
*------------------------------------------------------------------------------------------------------*/
#pragma pack(push)
#pragma pack(4)
struct NodeDescriptor
{
		  NodeDescriptor* _nextNode;						//next Node pointer
		  uint32_t 		  _nodeRefCounter;					//reference counter
		  bool nodeStatus;							        //is this node still exist in memory?
};
#pragma pack(pop)

template<typename _Tp,std::size_t _objectAmount>
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
	  	  uint32_t _invisibleBlockSize = sizeof(NodeDescriptor);			//The size of NodeDescriptor structure
		  uint32_t _allocateMemSpace = sizeof(_Tp);							//return the size of _Tp(sizeof(_Tp))

		  uint32_t _blockTotalSize = sizeof(NodeDescriptor) + sizeof(_Tp); 	//The total size of sizeof(NodeDescriptor) + sizeof(_Tp)
          uint32_t _objectpoolTotalSize = 0;
		  
		  void* 			_objectPoolMemBase;				    			//The base address of the whole objectpool
		  NodeDescriptor*   _nodeHeader = nullptr;							//The pointer points to the head of the NodeDescriptor

		  bool 				_objectPoolInitStatus;							//objectPool init status
		  std::mutex 		m_mutex;										//the mutex
};

template<typename _Tp,std::size_t _objectAmount>
HCNSObjectPoolAllocator<_Tp,_objectAmount>::HCNSObjectPoolAllocator()
{
	this->initObjectPool();
}

template<typename _Tp,std::size_t _objectAmount>
HCNSObjectPoolAllocator<_Tp,_objectAmount>::~HCNSObjectPoolAllocator()
{
		  ::delete[]this->_objectPoolMemBase;
}


/*------------------------------------------------------------------------------------------------------
* start to init memory pool
* @description: according to _Tp to calculate memory size
* @function:  void initObjectPool(std::size_t _objectAmount)
* @param: [IN] std::size_t _objectAmount
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp,std::size_t _objectAmount>
void HCNSObjectPoolAllocator<_Tp,_objectAmount>::initObjectPool()
{
	this->_objectpoolTotalSize = _objectAmount * this->_blockTotalSize;

	//calculate and allocate memory size by using memory pool
	this->_objectPoolMemBase = ::new char[this->_objectpoolTotalSize];

	//do the data type cast
	this->_nodeHeader = reinterpret_cast<NodeDescriptor*>(this->_objectPoolMemBase);
	this->_nodeHeader->_nodeRefCounter = 0;
	this->_nodeHeader->_nextNode = nullptr;
	this->_nodeHeader->nodeStatus = true;

	NodeDescriptor* prev = this->_nodeHeader;
	NodeDescriptor* _nextNode = reinterpret_cast<NodeDescriptor*>(
			reinterpret_cast<char*>(this->_objectPoolMemBase) + this->_blockTotalSize
			);
	NodeDescriptor* pend = reinterpret_cast<NodeDescriptor*>(
			reinterpret_cast<char*>(this->_objectPoolMemBase) + this->_objectpoolTotalSize
			);

	/*-----------------------------------------------------------------------------------*
		*		  		  		/\  |------NodeDescriptor---------|------------_Tp------------
		*		  		  		|   |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		*		  		  		|   |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		*	   _ObjPoolSize     |   |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		*		 				|   |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		* 					    |   |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		* 					    \/  |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		*------------------------------------------------------------------------------------
		*------------------------------------------------------------------------------------
		* in order to access to the next memoryblock, the offset of (char)pointer
		* should be the size of NodeDesciptor+_Tp
		*/

		/* traveral the pool and setup linklist connection!*/
	for (; _nextNode < pend; _nextNode = reinterpret_cast<NodeDescriptor*>(reinterpret_cast<char*>(_nextNode) + this->_blockTotalSize)){
		_nextNode->_nodeRefCounter = 0;
		_nextNode->nodeStatus = true;
		_nextNode->_nextNode = nullptr;

		prev->_nextNode = _nextNode;		//get next block
		prev = _nextNode;
	}
	
	this->_objectPoolInitStatus = true;
}
/*------------------------------------------------------------------------------------------------------
* alloc objects' memory according to it's size
* @description: according to object size and return an pointer which points to _Tp
* @function:  _Tp* allocObjectMemory(size_t _size)
* @param: [IN] size_t _size
* @retvalue: _Tp*
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp,std::size_t _objectAmount>
_Tp* HCNSObjectPoolAllocator<_Tp,_objectAmount>::allocObjectMemory(size_t _size)
{
		  NodeDescriptor* pAllocMem(nullptr);
		  /*
		  * if pool hasn't been inited and allocated with memory
		  */
		  if (!this->_objectPoolInitStatus || this->_objectPoolMemBase == nullptr) {
					this->initObjectPool();
		  }

		  /*
		  * there is no MemoryBlock header because of the pool is run out of free memory ,therefore
		  * create a temporary memory allocation
		  * Add a mutex lock in order to avoid multithreading problem
		  */
		  if (nullptr == this->getUnAmbigousHeaderValue()) {

					/*
					* memorypool needs to manage all the allocated memory,therefore
					* we need an additional descriptor size (sizeof(MemoryBlock)) add up to the totoal size
					*/
					//auto additional_size = _size + sizeof(NodeDescriptor);
					NodeDescriptor* pAdd = reinterpret_cast<NodeDescriptor*>(
							  ::new char[this->_blockTotalSize]
							  );

					pAdd->_nodeRefCounter = 1;
					pAdd->nodeStatus = false;
					pAdd->_nextNode = nullptr;

					pAllocMem = pAdd;								  //

		  }
		  else {
					std::lock_guard<std::mutex> _lckg(this->m_mutex);
					pAllocMem = this->_nodeHeader;										  //get empty memory block
					this->_nodeHeader = this->_nodeHeader->_nextNode;	  //move header to the next memory block
					pAllocMem->_nodeRefCounter = 1;											  //set this memory region reference time
		  }
		  /* !!! jump the memoryblock scale !!!*/
		  return reinterpret_cast<_Tp*>(++pAllocMem);
}

/*------------------------------------------------------------------------------------------------------
* free object's memory space
* @function:  void deallocObjectMemory( _Tp* _ptr)
* @param:[IN]  _Tp* _ptr
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp,std::size_t _objectAmount>
void HCNSObjectPoolAllocator<_Tp,_objectAmount>::deallocObjectMemory(_Tp* _ptr)
{
		  /*calculate pointer offset*/
		  NodeDescriptor* _pMemInfo = reinterpret_cast<NodeDescriptor*>(
					reinterpret_cast<char*>(_ptr) - sizeof(NodeDescriptor*)
					);

		  /*
		  * is the memory block inside the object pool?
		  * if so, recycle this memory block(just like insert into a linklist from the head)
		  */
		  if (_pMemInfo->nodeStatus) {
					/* see is there any other references to this object , the number should lower than 2*/
					if ((--_pMemInfo->_nodeRefCounter)) {				// reference still exist, memory pool can not recycle this memory block
							  return;
					}
					std::lock_guard<std::mutex> _lckg(this->m_mutex);
					_pMemInfo->_nextNode = this->_nodeHeader;
					this->_nodeHeader = _pMemInfo;
		  }
		  else { /*outside memory pool*/
					::delete[]_pMemInfo;
		  }
}

/*------------------------------------------------------------------------------------------------------
* get unambigous _nodeHeader value (meanwhile, mutex lock should be activated )
* @function:  NodeDescriptor* getUnAmbigousHeaderValue()
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp,std::size_t _objectAmount>
NodeDescriptor* HCNSObjectPoolAllocator<_Tp,_objectAmount>::getUnAmbigousHeaderValue()
{
	std::lock_guard<std::mutex> _lckg(this->m_mutex);
	return this->_nodeHeader;
}
#endif
