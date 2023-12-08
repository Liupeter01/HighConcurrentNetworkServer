#include<HCNSMemoryAllocator.hpp>

void* operator new(size_t _size)
{
	return MemoryAllocator::getInstance(
		MEMORYPOOL_BLOCK_COUNT,
		MEMORYPOOL_BLOCK_SZ8,
		MEMORYPOOL_BLOCK_SZ16,
		MEMORYPOOL_BLOCK_SZ32,
		MEMORYPOOL_BLOCK_SZ64,
		MEMORYPOOL_BLOCK_SZ128,
		MEMORYPOOL_BLOCK_SZ256,
		MEMORYPOOL_BLOCK_SZ512,
		MEMORYPOOL_BLOCK_SZ1024,
		MEMORYPOOL_BLOCK_SZ2048,
		MEMORYPOOL_BLOCK_SZ4096
	).allocPool(_size);
}

void operator delete(void* _ptr)
{
	if (_ptr != nullptr) {
		MemoryAllocator::getInstance(
			MEMORYPOOL_BLOCK_COUNT,
			MEMORYPOOL_BLOCK_SZ8,
			MEMORYPOOL_BLOCK_SZ16,
			MEMORYPOOL_BLOCK_SZ32,
			MEMORYPOOL_BLOCK_SZ64,
			MEMORYPOOL_BLOCK_SZ128,
			MEMORYPOOL_BLOCK_SZ256,
			MEMORYPOOL_BLOCK_SZ512,
			MEMORYPOOL_BLOCK_SZ1024,
			MEMORYPOOL_BLOCK_SZ2048,
			MEMORYPOOL_BLOCK_SZ4096
		).freePool(_ptr);
	}
}

void* operator new[](size_t _size)
{
	return ::operator new(_size);
}

void operator delete[](void* _ptr)
{
	::operator delete(_ptr);
}