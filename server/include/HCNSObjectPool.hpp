#pragma once
#ifndef _HCNSOBJECTPOOL_H_
#define _HCNSOBJECTPOOL_H_

#include<HCNSObjectPoolAllocator.hpp>	//introduce objectpool allocator

template<typename _Tp,std::size_t _objectAmount>
class HCNSObjectPool
{
public:
	typedef HCNSObjectPoolAllocator<_Tp,_objectAmount>  pool_allocator;
	typedef pool_allocator & allocator_ref;
	typedef _Tp value_type;
	typedef value_type* pointer;
public:
	void* operator new(size_t _size);
	void operator delete(void* _ptr);
	void* operator new[](size_t _size);
	void operator delete[](void* _ptr);

	template<typename ...Args>
	static _Tp* createObject(Args &&... args);

	static void deleteObject(_Tp* obj);

protected:
	HCNSObjectPool() = default;
	virtual ~HCNSObjectPool() =default;
	HCNSObjectPool(const HCNSObjectPool&) = delete;
	HCNSObjectPool&operator=(const HCNSObjectPool&) = delete;

private:
	static allocator_ref getInstance();
};

template<typename _Tp,std::size_t _objectAmount>
void* HCNSObjectPool<_Tp,_objectAmount>::operator new(size_t _size)
{
		  return getInstance().allocObjectMemory(_size);
}

template<typename _Tp,std::size_t _objectAmount>
void HCNSObjectPool<_Tp,_objectAmount>::operator delete(void* _ptr)
{
		  getInstance().deallocObjectMemory(reinterpret_cast< _Tp*>(_ptr));
}

template<typename _Tp,std::size_t _objectAmount>
void* HCNSObjectPool<_Tp,_objectAmount>::operator new[](size_t _size)
{
	return operator new(_size);
}

template<typename _Tp,std::size_t _objectAmount>
void HCNSObjectPool<_Tp,_objectAmount>::operator delete[](void* _ptr)
{
	operator delete(_ptr);
}

/*------------------------------------------------------------------------------------------------------
* create an object
* @description: use templates to forward init params to class
* @function:  _Tp *createObject(Args &&... args)
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp,std::size_t _objectAmount>
template<typename ...Args>
typename HCNSObjectPool<_Tp,_objectAmount>::pointer
HCNSObjectPool<_Tp,_objectAmount>::createObject(Args &&... args) 
{
		  _Tp* obj(new _Tp(args...));
		  return obj;
}

/*------------------------------------------------------------------------------------------------------
* delete an object
* @function:  _Tp *deleteObject(_Tp* obj)
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp,std::size_t _objectAmount>
void HCNSObjectPool<_Tp,_objectAmount>::deleteObject(_Tp* obj) 
{
		  delete obj;
}

/*------------------------------------------------------------------------------------------------------
* create an static singleton object instance
* @description: use singleton design pattern
* @function:  static HCNSObjectPoolAllocator<_Tp> &getInstance()
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp,std::size_t _objectAmount>
typename HCNSObjectPool<_Tp,_objectAmount>::allocator_ref
HCNSObjectPool<_Tp,_objectAmount>::getInstance()
{
	static pool_allocator alloc_obj;
	return alloc_obj;
}

#endif