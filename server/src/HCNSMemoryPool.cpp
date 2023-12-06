#include<HCNSMemoryPool.hpp>

MemoryPool::MemoryPool()
	:MemoryPool(0, 0)
{
}

MemoryPool::MemoryPool(uint32_t allocate_size,uint32_t blocks_count)
		  :_allocateMemSpace(allocate_size),_blocksCount(blocks_count)
{    
}

MemoryPool::~MemoryPool()
{
		  /*check is this->m_paddr nullptr*/
		  if (this->_poolMemBase != nullptr) {
					::free(this->_poolMemBase);
		  }
}

void MemoryPool::setAllocateSize(uint32_t size)
{
	this->_allocateMemSpace = size;
}
void MemoryPool::setBlocksCount(uint32_t count)
{
	this->_blocksCount  = count;
}

/*------------------------------------------------------------------------------------------------------
* start to init memory pool
* @function:  bool initMemoryPool
* @retvalue: bool
*------------------------------------------------------------------------------------------------------*/
bool MemoryPool::initMemoryPool()
{
			/*The total size of sizeof(MemoryBlock) + allocate_size*/
		    this->_blockTotalSize = this->_allocateMemSpace + this->_invisibleBlockSize;

		    /*The total size of the whole Pool(_blockTotalSize * _blocksCount)*/
		    this->_poolTotalSize = this->_blockTotalSize * _blocksCount;	 
		  	
			/* check the status of the initital, it should be work at the first time*/
			if (this->_mempoolInitStatus || nullptr != this->_poolMemBase) {
					return false;
			}

			/*allocate memory by using malloc*/
			this->_poolMemBase = reinterpret_cast<void*>(::malloc(this->_poolTotalSize));

			if (nullptr != this->_poolMemBase) {

						/*the first position of the memory block should be the head*/
						this->_blockHeader = reinterpret_cast<MemoryBlock*>(this->_poolMemBase);
						this->_blockHeader->_blockStatus = true;			//this block inside memory pool;
						this->_blockHeader->_blockID = _blockIDCounter++;
						this->_blockHeader->_blockRefCounter = 0;
						this->_blockHeader->_belongstoPool = this;
						this->_blockHeader->_nextBlock = nullptr;			//get next block


						MemoryBlock* prev = this->_blockHeader;
						MemoryBlock* pnext = reinterpret_cast<MemoryBlock*>(
								reinterpret_cast<char*>(this->_poolMemBase) + this->_blockTotalSize
								);
						MemoryBlock* pend = reinterpret_cast<MemoryBlock*>(
								reinterpret_cast<char*>(this->_poolMemBase) + (this->_poolTotalSize)
								);

						/* traveral the pool and setup linklist connection!*/
						for (; pnext < pend; pnext = reinterpret_cast<MemoryBlock*>(reinterpret_cast<char*>(pnext) + this->_blockTotalSize)) {				
								pnext->_blockStatus = true;			//this block inside memory pool;
								pnext->_blockID = _blockIDCounter++;
								pnext->_blockRefCounter = 0;
								pnext->_belongstoPool = this;
								pnext->_nextBlock = nullptr;	
								prev->_nextBlock = pnext;								//get next block
								prev = pnext;
						}
			}
			return (_mempoolInitStatus = true);
}

/*------------------------------------------------------------------------------------------------------
* Alloc memory for the memory pool(using system call command ::malloc)
* @function:  void* allocMem(size_t _size)
* @param :  [IN] size_t _size
* @retvalue: void*
*------------------------------------------------------------------------------------------------------*/
void* MemoryPool::allocMem(size_t _size)
{
		  MemoryBlock* pAllocMem(nullptr);
		  /*
		  * if pool hasn't been inited and allocated with memory
		  * then try to use malloc to allocate memory
		  */
		 
		  if (!this->_mempoolInitStatus|| this->_poolMemBase == nullptr) {
					this->initMemoryPool();
		  }

		  /*
		  * there is no MemoryBlock header because of the pool is run out of free memory,
		  * therefore create a temporary memory allocation
		  * Add a mutex lock in order to avoid multithreading problem
		  */
		  if (nullptr == this->getUnAmbigousHeaderValue()) {

					/*
					* memorypool needs to manage all the allocated memory,therefore
					* we need an additional descriptor size (sizeof(MemoryBlock)) add up to the total size
					*/
					MemoryBlock* pAdd = reinterpret_cast<MemoryBlock*>(::malloc(_size + sizeof(MemoryBlock)));
					if(nullptr == pAdd){
						return nullptr;
					}

					pAdd->_blockStatus = false;					  //this block is outside memory pool, so it doesn't belong to a pool;
					pAdd->_blockRefCounter = 1;
					pAdd->_belongstoPool = this;
					pAdd->_nextBlock = nullptr;				  	  //get next block

					pAllocMem = pAdd;							

		  }
		  else {
					std::lock_guard<std::mutex> _lckg(this->_mempoolLock);
					pAllocMem = this->_blockHeader;										
					this->_blockHeader = this->_blockHeader->_nextBlock;	  		//move header to the next memory block
					++pAllocMem->_blockRefCounter;									//set this memory region reference time
		  }
		  /* !!! jump the memoryblock scale !!!*/
		  return reinterpret_cast<void*>(++pAllocMem);
}

/*------------------------------------------------------------------------------------------------------
* freememory for the memory pool(using system call command ::free)
* @function:  void allocMem(void*_ptr)
* @param :  [IN] void* _ptr
*------------------------------------------------------------------------------------------------------*/
void MemoryPool::freeMem(void* _ptr)
{
		  /*calculate pointer offset*/
		  MemoryBlock* _pMemInfo = reinterpret_cast<MemoryBlock*>(
					reinterpret_cast<char*>(_ptr) - sizeof(MemoryBlock)
					);

		  /* 
		  * see is there any other references to this memory region, the number should lower than 2
		  * if reference still exist, memory pool can not recycle this memory block
		  */
		  if ((--_pMemInfo->_blockRefCounter)) {
					return;
		  }

		  /*
		  * is the memory block inside the memory pool?
		  * if so, recycle this memory block(just like insert into a linklist from the head)
		  */
		  if (_pMemInfo->_blockStatus) {
					std::lock_guard<std::mutex> _lckg(this->_mempoolLock);
					_pMemInfo->_nextBlock = this->_blockHeader;
					this->_blockHeader = _pMemInfo;
		  }
		  else { /*outside memory pool*/
					::free(_pMemInfo);
		  }
}

/*------------------------------------------------------------------------------------------------------
* get unambigous m_pHeader value (meanwhile, mutex lock should be activated )
* @function:  MemoryBlock* getUnAmbigousHeaderValue()
* @param :  [IN] T _ptr
*------------------------------------------------------------------------------------------------------*/
MemoryBlock* MemoryPool::getUnAmbigousHeaderValue()
{
		  std::lock_guard<std::mutex> _lckg(this->_mempoolLock);
		  return this->_blockHeader;
}