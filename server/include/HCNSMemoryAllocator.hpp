#pragma once
#ifndef _HCNSMEMORYPOOL_H_
#define _HCNSMEMORYPOOL_H_
#include<mutex>
#include<HCNSMemoryBlock.hpp>

/*---------------------------------------------------------------------------------------------------
* class MemoryPool
* UserSpace: expect MemoryBlock, this space is designed to allocate for the user
* TupleLines: the total lines of the array
* ---------------------------------------------------------------------------------------------------*/
class MemoryPool
{
public:
		  MemoryPool();
		  MemoryPool(
					uint32_t _blockSize = sizeof(MemoryBlock),
					void* _pAddr = nullptr
		  );
		  MemoryPool(
					uint32_t _userspace,
					uint32_t _tupleLines,
					uint32_t _blockSize = sizeof(MemoryBlock),
					void* _pAddr = nullptr
		  );
		  virtual ~MemoryPool();

public:
		  bool initMemoryPool();
		  template<typename T> T allocMem(size_t _size);
		  template<typename T> void freeMem(T _ptr);

private:
		  MemoryBlock* getUnAmbigousHeaderValue();

private:
		  /*-----------------------------------------------------------------------------------*
		   *		  		  		/\   |------MemoryBlock---------|------UserSpace---------
		   *		  		  		 |   |---sizeof(MemoryBlock)---|------m_userSpace-------
		   *		  		  		 |   |---sizeof(MemoryBlock)---|------m_userSpace-------
		   *m_tupleLines  |   |---sizeof(MemoryBlock)---|------m_userSpace-------
		   *		  		  		 |   |---sizeof(MemoryBlock)---|------m_userSpace-------
		   *		  		  		 |   |---sizeof(MemoryBlock)---|------m_userSpace-------
		   *		 		  		\/   |---sizeof(MemoryBlock)---|------m_userSpace-------
		   *------------------------------------------------------------------------------------
		   *----------------------------------m_tupleTotalSize-----------------------------
		   *------------------------------------------------------------------------------------*/
		  uint32_t m_blockSize;													//the size of each memory block to record info
		  uint32_t m_userSpace;													//the size for user
		  uint32_t m_tupleLines;
		  uint32_t m_tupleTotalSize;											//the size of a tuple(including sizeof(MemoryBlock) and m_userSpace)
		  uint32_t m_poolTotalSize;										    //the total size of the all memory pool

		  std::mutex m_memoryLock;										    //mutex lock for allocate and deallocate
		  bool m_initStatus;															//the init status of memorypool
		  void* m_pAddr;														    //the address of memory pool
		  MemoryBlock* m_pHeader;									    //pheader pointer points to the head of memory block
};

template<size_t UserSpace, size_t TupleLines>
class MemoryAllocator : public MemoryPool
{
public:
		  MemoryAllocator()
					:MemoryPool(UserSpace, TupleLines, sizeof(MemoryBlock))
		  {
		  }
};

#endif

/*------------------------------------------------------------------------------------------------------
* Alloc memory for the memory pool(using system call command ::malloc)
* @function:  T allocMem(size_t _size)
* @param :  [IN] size_t _size
* @retvalue: T
*------------------------------------------------------------------------------------------------------*/
template<typename T> T MemoryPool::allocMem(size_t _size)
{
		  MemoryBlock* pAllocMem(nullptr);
		  /*
		  * if pool hasn't been inited and allocated with memory
		  * then try to use malloc to allocate memory
		  */
		  if (!this->m_initStatus || this->m_pAddr == nullptr) {
					this->initMemoryPool();
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
					auto additional_size = _size + sizeof(MemoryBlock);
					MemoryBlock* pAdd = reinterpret_cast<MemoryBlock*>(::malloc(additional_size));

					pAdd->setBlockStatus(false);					  //this block outside memory pool;
					pAdd->setBlockID(0);
					pAdd->setBlockRef(1);
					pAdd->setBlockRelatePool(this);
					pAdd->setNextBlock(nullptr);				  //get next block

					pAllocMem = pAdd;								  //

		  }
		  else {
					std::lock_guard<std::mutex> _lckg(this->m_memoryLock);
					pAllocMem = this->m_pHeader;										  //get empty memory block
					this->m_pHeader = this->m_pHeader->getNextBlock();	  //move header to the next memory block
					pAllocMem->setBlockRef(1);											  //set this memory region reference time
		  }
		  /* !!! jump the memoryblock scale !!!*/
		  return reinterpret_cast<T>(++pAllocMem);
}

/*------------------------------------------------------------------------------------------------------
* freememory for the memory pool(using system call command ::free)
* @function:  void allocMem(T_ptr)
* @param :  [IN] T _ptr
*------------------------------------------------------------------------------------------------------*/
template<typename T> void MemoryPool::freeMem(T _ptr)
{
		  /*calculate pointer offset*/
		  MemoryBlock* _pMemInfo = reinterpret_cast<MemoryBlock*>(
					reinterpret_cast<char*>(_ptr) - sizeof(MemoryBlock)
					);

		  /* see is there any other references to this memory region, the number should lower than 2*/
		  if ((--_pMemInfo->getBlockRef())) {				// reference still exist, memory pool can not recycle this memory block
					return;
		  }
		  /*
		  * is the memory block inside the memory pool?
		  * if so, recycle this memory block(just like insert into a linklist from the head)
		  */
		  if (_pMemInfo->getBlockStatus()) {
					std::lock_guard<std::mutex> _lckg(this->m_memoryLock);
					_pMemInfo->setNextBlock(this->m_pHeader);
					this->m_pHeader = _pMemInfo;
		  }
		  else { /*outside memory pool*/
					::free(_pMemInfo);
		  }
}
