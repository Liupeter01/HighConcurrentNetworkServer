#include<HCNSObjectPool.hpp>

template<typename _Tp, std::size_t _ObjPoolSize>
HCNSObjectPool<_Tp,_ObjPoolSize> ::HCNSObjectPool()
{
}

template<typename _Tp, std::size_t _ObjPoolSize>
HCNSObjectPool<_Tp,_ObjPoolSize>::~HCNSObjectPool()
{
}

template<typename _Tp, std::size_t _ObjPoolSize>
void* HCNSObjectPool<_Tp, _ObjPoolSize>::operator new(size_t _size)
{
		  return getInstance().allocObjectMemory(_size);
}

template<typename _Tp, std::size_t _ObjPoolSize>
void HCNSObjectPool<_Tp, _ObjPoolSize>::operator delete(void* _ptr)
{
		  getInstance().deallocObjectMemory(reinterpret_cast< _Tp*>(_ptr));
}

template<typename _Tp, std::size_t _ObjPoolSize>
void* HCNSObjectPool<_Tp,_ObjPoolSize>::operator new[](size_t _size)
{
	return operator new(_size);
}

template<typename _Tp, std::size_t _ObjPoolSize>
void HCNSObjectPool<_Tp,_ObjPoolSize>::operator delete[](void* _ptr)
{
	operator delete(_ptr);
}

/*------------------------------------------------------------------------------------------------------
* create an object
* @description: use templates to forward init params to class
* @function:  _Tp *createObject(Args &&... args)
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp, std::size_t _ObjPoolSize>
template<typename ...Args>
_Tp* HCNSObjectPool<_Tp,_ObjPoolSize>::createObject(Args &&... args) {
		  _Tp* obj(new _Tp(args...));
		  return obj;
}

/*------------------------------------------------------------------------------------------------------
* delete an object
* @function:  _Tp *deleteObject(_Tp* obj)
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp, std::size_t _ObjPoolSize>
void  HCNSObjectPool<_Tp,_ObjPoolSize>::deleteObject(_Tp* obj) {
		  delete obj;
}

/*------------------------------------------------------------------------------------------------------
* create an static singleton object instance
* @description: use singleton design pattern
* @function:  static HCNSObjectPoolAllocator<_Tp,_ObjPoolSize> &getInstance()
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp, std::size_t _ObjPoolSize>
HCNSObjectPoolAllocator<_Tp,_ObjPoolSize> & HCNSObjectPool<_Tp,_ObjPoolSize>::getInstance()
{
	static allocator alloc_obj;
	return alloc_obj;
}