#pragma once
#ifndef _HCNSOBJECTPOOL_H_
#define _HCNSOBJECTPOOL_H_

#include<HCNSObjectPoolAllocator.hpp>	//introduce objectpool allocator
#include "../src/HCNSObjectPoolAllocator.cpp"

template<typename _Tp, std::size_t _ObjPoolSize>
class HCNSObjectPool
{
	typedef HCNSObjectPoolAllocator<_Tp,_ObjPoolSize> allocator;

public:
	void* operator new(size_t _size);
	void operator delete(void* _ptr);
	void* operator new[](size_t _size);
	void operator delete[](void* _ptr);

	template<typename ...Args>
	static _Tp* createObject(Args &&... args);

	static void deleteObject(_Tp* obj);

protected:
	HCNSObjectPool();
	virtual ~HCNSObjectPool();

private:
	static HCNSObjectPoolAllocator<_Tp,_ObjPoolSize> &getInstance();
};
#endif