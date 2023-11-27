#pragma once
#ifndef _HCNSMEMMANAGEMENT_H_
#define _HCNSMEMMANAGEMENT_H_

#include<HCNSMemoryAllocator.hpp>
/*
 *detour global memory allocation and deallocation functions
 *use library from HCNSMemoryPool
 */
void* operator new(size_t _size);
void operator delete(void* _ptr);
void* operator new[](size_t _size);
void operator delete[](void* _ptr);

/*------------------------------------------------------------------------------------------------------
* MemoryManagement
*------------------------------------------------------------------------------------------------------*/
class MemoryManagement
{
private:
          MemoryManagement();
          MemoryManagement(const uint32_t _tupleLines, const std::initializer_list<uint32_t> _list);
          MemoryManagement(const std::initializer_list<std::pair<uint32_t, uint32_t>>_list);
          MemoryManagement(const MemoryManagement& mem);
          virtual ~MemoryManagement();

public:
          static MemoryManagement& getInstance();
		  template<typename T> T allocPool(size_t _size); 
		  template<typename T> void freePool(T _ptr); 

private:
		  void initMemoryAlloc(
					std::array< MemoryPool*, 4096 >::iterator _ib,
					std::array< MemoryPool*, 4096 >::iterator _ie,
					MemoryPool* _ptr
		  );

          uint32_t getLargestBlockSize();

private:
		  uint32_t m_maximumMemory = 4096;	//maxinum memory size support

		  MemoryAllocator<64, 100000> m_mem64;	//64B memory space for user, only create 100000 tuple lines
		  MemoryAllocator<128, 100000> m_mem128;	//128B memory space for user, only create 100000 tuple lines
		  MemoryAllocator<256, 100000> m_mem256;	//256B memory space for user, only create 100000 tuple lines
		  MemoryAllocator<512, 100000> m_mem512;	//512B memory space for user, only create 100000 tuple lines
		  MemoryAllocator<1024, 100000> m_mem1024;	//1024B memory space for user, only create 100000 tuple lines
		  MemoryAllocator<2048, 100000> m_mem2048;	//2048B memory space for user, only create 100000 tuple lines
		  MemoryAllocator<4096, 100000> m_mem4096;	//4096B memory space for user, only create 100000 tuple lines

		  std::array<MemoryPool*, 4096> m_memMatching;	//create an array of pointer to match memory units
};

/*------------------------------------------------------------------------------------------------------
* Alloc memory for the memory pool(using system call command ::malloc)
* @function:  T allocPool(size_t _size)
* @param :  [IN] size_t _size
* @retvalue: T
*------------------------------------------------------------------------------------------------------*/
template<typename T> T MemoryManagement::allocPool(size_t _size)
{
          /*invalid memory allocation size*/
          if (_size > 0) {

                    /*user desired memory size lower than or equal to one tuple size*/
                    if (_size <= this->m_maximumMemory) {

                              /* access to the specific memory pool which satisfied the size requirement */
                              return this->m_memMatching[_size - 1]->allocMem<T>(_size);
                    }
                    else {
                              /*
                               * user desired memory much more higher than the memory allocator
                               * this memory area should be allocated by using memory allocation system call
                               */
                              MemoryBlock* pallocPool = reinterpret_cast<MemoryBlock*>(::malloc(_size + sizeof(MemoryBlock)));
                              pallocPool->setBlockStatus(false);			                //this block outside memory pool;
                              pallocPool->setBlockID(0);
                              pallocPool->setBlockRef(1);
                              pallocPool->setBlockRelatePool(nullptr);              //this memory area doesn't belongs to any memory pool
                              pallocPool->setNextBlock(nullptr);			            //get next block
                              return reinterpret_cast<T>(++pallocPool);
                    }
          }
          return nullptr;                          //invalid memory allocation
}

/*------------------------------------------------------------------------------------------------------
* freememory for the memory pool(using system call command ::free)
* @function:  void allocMem(T_ptr)
* @param :  [IN] T _ptr
*------------------------------------------------------------------------------------------------------*/
template<typename T> void MemoryManagement::freePool(T _ptr)
{
          /*calculate pointer offset*/
          MemoryBlock* _pMemInfo = reinterpret_cast<MemoryBlock*>(
                    reinterpret_cast<char*>(_ptr) - sizeof(MemoryBlock)
                    );

          /*judge whether this memory block belongs to specfic memory pool*/
          if (_pMemInfo->getBlockStatus()) {
                    _pMemInfo->getMemoryPool()->freeMem(_ptr);
          }
          else { /*outside memory pool*/
                    if (!(--_pMemInfo->getBlockRef())) {
                              ::free(_pMemInfo);
                    }
          }
}

#endif