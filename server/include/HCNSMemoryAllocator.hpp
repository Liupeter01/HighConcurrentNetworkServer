#pragma once
#ifndef _HCNSMEMMANAGEMENT_H_
#define _HCNSMEMMANAGEMENT_H_

#include<HCNSMemoryPool.hpp>

/*
    the available size of memory pool is : 8-4096
    8 16 32 64 128 256 512 1024 2048 4096
*/
#define POOL_MAXINUM_SIZE 10                      //how many pools are managed by MemoryAllocator
#define MEMORYPOOL_BLOCK_COUNT 1000               //how many blocks are created per one size
#define MEMORYPOOL_BLOCK_SZ8 (8)
#define MEMORYPOOL_BLOCK_SZ16 (16)
#define MEMORYPOOL_BLOCK_SZ32 (32)
#define MEMORYPOOL_BLOCK_SZ64 (64)
#define MEMORYPOOL_BLOCK_SZ128 (128)
#define MEMORYPOOL_BLOCK_SZ256 (256)
#define MEMORYPOOL_BLOCK_SZ512 (512)
#define MEMORYPOOL_BLOCK_SZ1024 (1024)
#define MEMORYPOOL_BLOCK_SZ2048 (2048)
#define MEMORYPOOL_BLOCK_SZ4096 (4096)

/*------------------------------------------------------------------------------------------------------
* MemoryAllocator
*------------------------------------------------------------------------------------------------------*/
class MemoryAllocator
{
protected:
          MemoryAllocator()= default;
          
          MemoryAllocator(const MemoryAllocator &) = delete;
          MemoryAllocator& operator=(const MemoryAllocator&) = delete;

          template<typename ...MemorySpaceArgs>
          MemoryAllocator(uint32_t blocks_count,MemorySpaceArgs ...args);

private:
         /*this function is the terminate condition for the recursive ctor function*/
         void createMemoryManageObject(uint32_t blocks_count){}

         template<typename Value, typename ...MemorySpaceArgs>
         void createMemoryManageObject(uint32_t blocks_count, Value t, MemorySpaceArgs ... args);
        
         inline int findMappingIndex(const size_t _size);

public:
          template<typename ...MemorySpaceArgs>
          static MemoryAllocator& getInstance(uint32_t blocks_count,MemorySpaceArgs ... args);

          void * allocPool(size_t _size); 
		  void freePool(void* _ptr); 

private:
         uint32_t retrieveMaxMemorySize = 0;                      //get the maxinum memory block size
         uint32_t _poolSizeArray[POOL_MAXINUM_SIZE];              //record an index to specifc memory pool(_poolSizeMapping)
         uint32_t _poolSizePos = 0;                               //record memory poolsize array maxinum pos
         
         MemoryPool _poolSizeMapping[POOL_MAXINUM_SIZE];
};

template<typename ...MemorySpaceArgs>
MemoryAllocator::MemoryAllocator(uint32_t blocks_count,MemorySpaceArgs ...args)
{
    this->createMemoryManageObject(blocks_count,args...);
}

/*------------------------------------------------------------------------------------------------------
* according to MemoryAllocator ctor's arguments to create data structure.
* @function: void createMemoryManageObject(uint32_t blocks_count,Value t,MemorySpaceArgs ...args)
* @description: variadic template
* @param : 1.[IN]uint32_t blocks_count
           2.[IN]Value t
           3.[IN]MemorySpaceArgs ...args
*------------------------------------------------------------------------------------------------------*/
template<typename Value, typename ...MemorySpaceArgs>
void MemoryAllocator::createMemoryManageObject(uint32_t blocks_count, Value t,MemorySpaceArgs ... args) 
{
   /*this->_poolSizePos should never larger than bound!!!*/
   if(this->_poolSizePos != POOL_MAXINUM_SIZE){
        this->_poolSizeArray[this->_poolSizePos] = static_cast<uint32_t>(t);
        this->_poolSizeMapping[this->_poolSizePos].setAllocateSize(static_cast<uint32_t>(t));
        this->_poolSizeMapping[this->_poolSizePos++].setBlocksCount(static_cast<uint32_t>(blocks_count));

        /*find the max memory block size*/
        if(this->retrieveMaxMemorySize <= static_cast<uint32_t>(t)){            
                this->retrieveMaxMemorySize = static_cast<uint32_t>(t);
        }
        this->createMemoryManageObject(blocks_count,args...);
   }
}

/*------------------------------------------------------------------------------------------------------
* get the singleton MemoryAllocator instance
* @function: MemoryAllocator& getInstance(uint32_t blocks_count,MemorySpaceArgs ... args)
* @param: 1.[IN] uint32_t blocks_count
          2.[IN] MemorySpaceArgs ... args

* @retvalue: MemoryAllocator&
*------------------------------------------------------------------------------------------------------*/
template<typename ...MemorySpaceArgs>
MemoryAllocator& MemoryAllocator::getInstance(uint32_t blocks_count,MemorySpaceArgs ... args)
{
    /* we pack our arguments in a MemoryAllocator&() function,the bind is there to avoid some gcc bug */
    static MemoryAllocator instance(blocks_count,args...);
    return instance;
}

#endif