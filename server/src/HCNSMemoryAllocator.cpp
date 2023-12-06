#include<HCNSMemoryAllocator.hpp>

/*------------------------------------------------------------------------------------------------------
* searching in the array in order to find the mapping index
* @function: uint32_t *findMappingIndex(const size_t _size)
* @param : [IN]size_t _size
* @retvalue: int
*------------------------------------------------------------------------------------------------------*/
inline int MemoryAllocator::findMappingIndex(const size_t _size)
{
        auto _result =  std::find_if(
                this->_poolSizeArray,
                this->_poolSizeArray + POOL_MAXINUM_SIZE,
                [_size](uint32_t num){
                        return ((_size <= num));
                }
        );
        if(_result != this->_poolSizeArray + POOL_MAXINUM_SIZE){
                return _result - this->_poolSizeArray;
        }
        else{
                return -1;
        }
}

/*------------------------------------------------------------------------------------------------------
* Alloc memory for the memory pool(using system call command ::malloc)
* @function: void * allocPool(size_t _size)
* @param :  [IN] size_t _size
* @retvalue: void *
*------------------------------------------------------------------------------------------------------*/
void * MemoryAllocator::allocPool(size_t _size)
{
            /*invalid memory allocation size*/
            if (_size <= 0) {
                    return nullptr;                          //invalid memory allocation
            }
        
            /* access to the specific memory pool which satisfied the size requirement */
            int _poolMappingindicator = this->findMappingIndex(_size);

            if(_poolMappingindicator){
                    return this->_poolSizeMapping[_poolMappingindicator].allocMem(_size);
            }
            else{
                    /*
                    * user desired memory much more higher than the memory allocator
                    * this memory area should be allocated by using memory allocation system call
                    */
                    MemoryBlock* pallocPool = reinterpret_cast<MemoryBlock*>(::malloc(_size + sizeof(MemoryBlock)));
                    pallocPool->_blockStatus = false;			    //this block outside memory pool;
                    pallocPool->_blockID = -1;
                    pallocPool->_blockRefCounter = 1;
                    pallocPool->_belongstoPool = nullptr;         //this memory area doesn't belongs to any memory pool
                    pallocPool->_nextBlock = nullptr;			    //get next block

                    return reinterpret_cast<void*>(++pallocPool);
            }
          
}

/*------------------------------------------------------------------------------------------------------
* freememory for the memory pool(using system call command ::free)
* @function:  void allocMem(void* _ptr)
* @param :  [IN] void* 
*------------------------------------------------------------------------------------------------------*/
void MemoryAllocator::freePool(void* _ptr)
{
          /*calculate pointer offset*/
          MemoryBlock* _pMemInfo = reinterpret_cast<MemoryBlock*>(
                reinterpret_cast<char*>(_ptr) - sizeof(MemoryBlock)
          );

          /*judge whether this memory block belongs to specfic memory pool*/
          if (_pMemInfo->_blockStatus) {
                    _pMemInfo->_belongstoPool->freeMem(_ptr);
          }
          else { /*outside memory pool*/
                    if (!(--_pMemInfo->_blockRefCounter)) {
                              ::free(_pMemInfo);
                    }
          }
}
