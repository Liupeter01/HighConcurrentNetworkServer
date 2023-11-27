#include<HCNSMemoryManagement.hpp>

/*detour global memory allocation and deallocation functions*/
void* operator new(size_t _size)
{
	return MemoryManagement::getInstance().allocPool<void*>(_size);
}

void operator delete(void* _ptr)
{
	if (_ptr != nullptr) {
		MemoryManagement::getInstance().freePool<void*>(_ptr);
	}
}

void* operator new[](size_t _size)
{
	return ::operator new(_size);
}

void operator delete[](void* _ptr)
{
	operator delete(_ptr);
}

/*------------------------------------------------------------------------------------------------------
* get the singleton memoryManagement instance
* @function:  MemoryManagement *getInstance
* @retvalue: MemoryManagement*
*------------------------------------------------------------------------------------------------------*/
MemoryManagement& MemoryManagement::getInstance()
{
          static MemoryManagement instance;
          return instance;
}

/*------------------------------------------------------------------------------------------------------
* setup static memorypool by using memoryallocator class
* @function:  MemoryManagement
*------------------------------------------------------------------------------------------------------*/
MemoryManagement::MemoryManagement()
{
          this->initMemoryAlloc(this->m_memMatching.begin() + 0, this->m_memMatching.begin() + 64, &this->m_mem64);
          this->initMemoryAlloc(this->m_memMatching.begin() + 64, this->m_memMatching.begin() + 128, &this->m_mem128);
          this->initMemoryAlloc(this->m_memMatching.begin() + 128, this->m_memMatching.begin() + 256, &this->m_mem256);
          this->initMemoryAlloc(this->m_memMatching.begin() + 256, this->m_memMatching.begin() + 512, &this->m_mem512);
          this->initMemoryAlloc(this->m_memMatching.begin() + 512, this->m_memMatching.begin() + 1024, &this->m_mem1024);
          this->initMemoryAlloc(this->m_memMatching.begin() + 1024, this->m_memMatching.begin() + 2048, &this->m_mem2048);
          this->initMemoryAlloc(this->m_memMatching.begin() + 2048, this->m_memMatching.begin() + 4096, &this->m_mem4096);
}

/*------------------------------------------------------------------------------------------------------
* setup static memorypool by using memoryallocator class
* @function:  MemoryManagement(const uint32_t _tupleLines, std::initializer_list<uint32_t> _list)
* @param: 1.const uint32_t _tupleLines
*                 2.std::initializer_list<uint32_t> _list
*------------------------------------------------------------------------------------------------------*/
MemoryManagement::MemoryManagement(
          const uint32_t _tupleLines,
          std::initializer_list<uint32_t> _list)
{
          for (auto ib = _list.begin(); ib != _list.end(); ++ib) {
                    //m_memPool.emplace_back((*ib), _tupleLines);
          }
}

/*------------------------------------------------------------------------------------------------------
* setup static memorypool by using memoryallocator class
* @function:  MemoryManagement(std::initializer_list<std::pair<uint32_t, uint32_t>>_list)
* @param: 1.std::initializer_list<std::pair<uint32_t, uint32_t>>_list
*------------------------------------------------------------------------------------------------------*/
MemoryManagement::MemoryManagement(std::initializer_list<std::pair<uint32_t, uint32_t>>_list)
{
          for (auto ib = _list.begin(); ib != _list.end(); ++ib) {
                    //m_memPool.emplace_back(ib->first, ib->second);
          }
}

/*------------------------------------------------------------------------------------------------------
* setup static memorypool by using memoryallocator class
* @function:  MemoryManagement(unsigned long long _tupleLines,std::initializer_list<unsigned long long> _list)
* @param: 1.unsigned long long _tupleLines
*                 2.std::initializer_list<unsigned long long> _list
*------------------------------------------------------------------------------------------------------*/
MemoryManagement::~MemoryManagement()
{

}

/*---------------------------------------------------------------------------------------------------------
* init memory mapping array of pointers
* @function:  void initMemoryAlloc
* @param: 1. std::array< MemoryPool*, 4096 >::iterator  &_ib
                   2. std::array< MemoryPool*, 4096 >::iterator  &_ie
                   3.MemoryPool* _ptr
*------------------------------------------------------------------------------------------------------*/
void MemoryManagement::initMemoryAlloc(
          std::array< MemoryPool*, 4096 >::iterator _ib,
          std::array< MemoryPool*, 4096 >::iterator _ie,
          MemoryPool* _ptr)
{
          for (auto ib = _ib; ib != _ie; ib++) {
                    (*ib) = _ptr;
          }
}

/*---------------------------------------------------------------------------------------------------------
* get the largest storage value in memory pool
* @function:  uint32_t getLargestBlockSize
* @retvalue: uint32_t 
*------------------------------------------------------------------------------------------------------*/
uint32_t MemoryManagement::getLargestBlockSize()
{
          return 0;
}