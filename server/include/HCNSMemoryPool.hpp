#pragma once
#ifndef _HCNSMEMORYPOOL_H_
#define _HCNSMEMORYPOOL_H_
#include<mutex>
#include<HCNSMemoryBlock.hpp>

/*---------------------------------------------------------------------------------------------------
* class MemoryPool
* @description: MemoryBlock structure is invisible to users
* @param:	
		  1. allocate_size: return the visible memory space for users
		  2. blocks_count:  how many memory blocks users want to create
* ---------------------------------------------------------------------------------------------------*/
class MemoryPool
{
public:
		  MemoryPool()
				:MemoryPool(0, 0)
		  {}
		  MemoryPool(uint32_t allocate_size,uint32_t blocks_count)
		  		  :_allocateMemSpace(allocate_size),_blocksCount(blocks_count)
		  {}

		  virtual ~MemoryPool(){
				if (this->_poolMemBase != nullptr) 
					::free(this->_poolMemBase);
		  }

		  void setAllocateSize(uint32_t size){this->_allocateMemSpace = size;}
		  void setBlocksCount(uint32_t count){this->_blocksCount  = count;}
		  bool initMemoryPool();
		  void* allocMem(size_t _size);
		  void freeMem(void* _ptr);

private:
		  MemoryBlock* getUnAmbigousHeaderValue();

private:
		  uint32_t _blockIDCounter = 0;						 	//the block id counter

		  /*--------------------------------------------------------------------------------------
		   *	 		  		/\   |------MemoryBlock--------|------UserSpace-------------|
		   *	 		  		 |   |---sizeof(MemoryBlock)---|------allocate_size---------|
		   *	 		  		 |   |---sizeof(MemoryBlock)---|------allocate_size---------|
		   *	 _blocksCount    |   |---sizeof(MemoryBlock)---|------allocate_size---------|
		   *	  		  		 |   |---sizeof(MemoryBlock)---|------allocate_size---------|
		   *	  		  		 |   |---sizeof(MemoryBlock)---|------allocate_size---------|
		   *	 		  		\/   |---sizeof(MemoryBlock)---|------allocate_size---------|
		   *--------------------------------------------------------------------------------------
		   *-------------------------|<----------------m_tupleTotalSize-------------------->|
		   */
		  uint32_t _invisibleBlockSize = sizeof(MemoryBlock);	//The size of MemoryBlock structure
		  uint32_t _allocateMemSpace;							//return the visible memory space for users
		  uint32_t _blocksCount;								//how many memory blocks() users want to create

		  uint32_t _blockTotalSize;							    //The total size of sizeof(MemoryBlock) + allocate_size
		  uint32_t _poolTotalSize;								//The total size of the whole Pool(_blockTotalSize * _blocksCount)
		  
		  void* _poolMemBase;				    				//The base address of the whole memory pool
		  MemoryBlock *_blockHeader = nullptr;					//The pointer points to the head of the MemoryBlock

		  bool _mempoolInitStatus = false;						//The init status of the memory pool
		  std::mutex _mempoolLock;								//mutex lock for multithread consideration
};

#endif