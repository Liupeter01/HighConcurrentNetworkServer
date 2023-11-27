#include<HCNSMemoryAllocator.hpp>

MemoryPool::MemoryPool()
		  :MemoryPool(sizeof(MemoryBlock), nullptr)
{
}

MemoryPool::MemoryPool(
		  uint32_t _blockSize,
		  void* _pAddr)
		  : MemoryPool(0, 0, sizeof(MemoryBlock), nullptr)
{
}

MemoryPool::MemoryPool(
		  uint32_t _userspace,
		  uint32_t _tupleLines,
		  uint32_t _blockSize,
		  void* _pAddr)
		  :m_userSpace(_userspace),
		  m_tupleLines(_tupleLines),
		  m_pAddr(_pAddr),
		  m_pHeader(nullptr),
		  m_blockSize(_blockSize),
		  m_initStatus(false)
{
		  this->m_tupleTotalSize = this->m_userSpace + this->m_blockSize;				  //the size of a tuple
		  this->m_poolTotalSize = this->m_tupleTotalSize * this->m_tupleLines;	      //the total size of the all memory pool
}

MemoryPool::~MemoryPool()
{
		  /*check is this->m_paddr nullptr*/
		  if (this->m_pAddr != nullptr) {
					::free(this->m_pAddr);
		  }
}

/*------------------------------------------------------------------------------------------------------
* start to init memory pool
* @function:  bool initMemoryPool
* @retvalue: bool
*------------------------------------------------------------------------------------------------------*/
bool MemoryPool::initMemoryPool()
{
		  /* check the status of the initital, it should be work at the first time*/
		  if (!this->m_initStatus || nullptr == this->m_pAddr) {

					/*
					* calculate the total size of the required memory
					* this->m_tupleTotalSize = this->m_tupleLines * (this->m_userSpace + this->m_blockSize)
					*/
					this->m_pAddr = reinterpret_cast<void*>(::malloc(this->m_poolTotalSize));

					if (nullptr != this->m_pAddr) {

							  /*the first position of the memory block should be the head*/
							  this->m_pHeader = reinterpret_cast<MemoryBlock*>(this->m_pAddr);
							  this->m_pHeader->setBlockStatus(true);			//this block inside memory pool;
							  this->m_pHeader->setBlockID(0);
							  this->m_pHeader->setBlockRef(0);
							  this->m_pHeader->setBlockRelatePool(this);
							  this->m_pHeader->setNextBlock(nullptr);			//get next block

							  /*-----------------------------------------------------------------------------------*
							   *		  		  		/\   |------MemoryBlock---------|------UserSpace---------
							   *		  		  		 |   |---sizeof(MemoryBlock)---|------m_userSpace-------
							   *		  		  		 |   |---sizeof(MemoryBlock)---|------m_userSpace-------
							   *m_tupleLines |   |---sizeof(MemoryBlock)---|------m_userSpace-------
							   *		  		  		 |   |---sizeof(MemoryBlock)---|------m_userSpace-------
							   *		  		  		 |   |---sizeof(MemoryBlock)---|------m_userSpace-------
							   *		 		  		\/   |---sizeof(MemoryBlock)---|------m_userSpace-------
							   *------------------------------------------------------------------------------------
							   *----------------------------------m_tupleTotalSize-----------------------------
							   *------------------------------------------------------------------------------------
							   * in order to access to the next memoryblock, the offset of (char)pointer
							   * should be the size of memoryblock + userspace
							   */
							  MemoryBlock* prev = this->m_pHeader;
							  MemoryBlock* pnext = reinterpret_cast<MemoryBlock*>(
										reinterpret_cast<char*>(this->m_pAddr) + this->m_tupleTotalSize
										);
							  MemoryBlock* pend = reinterpret_cast<MemoryBlock*>(
										reinterpret_cast<char*>(this->m_pAddr) + (this->m_poolTotalSize)
										);

							  /* traveral the pool and setup linklist connection!*/
							  for (; pnext < pend; pnext = reinterpret_cast<MemoryBlock*>(reinterpret_cast<char*>(pnext) + this->m_tupleTotalSize)) {
										pnext->setBlockStatus(true);			//this block inside memory pool;
										pnext->setBlockID(0);
										pnext->setBlockRef(0);
										pnext->setBlockRelatePool(this);
										pnext->setNextBlock(nullptr);
										prev->setNextBlock(pnext);								//get next block
										prev = pnext;
							  }
					}
					return (this->m_initStatus = true);
		  }
		  return false;
}

/*------------------------------------------------------------------------------------------------------
* get unambigous m_pHeader value (meanwhile, mutex lock should be activated )
* @function:  MemoryBlock* getUnAmbigousHeaderValue()
* @param :  [IN] T _ptr
*------------------------------------------------------------------------------------------------------*/
MemoryBlock* MemoryPool::getUnAmbigousHeaderValue()
{
		  MemoryBlock* _retValue(nullptr);
		  {
					std::lock_guard<std::mutex> _lckg(this->m_memoryLock);
					_retValue = this->m_pHeader;
		  }
		  return _retValue;
}