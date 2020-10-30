#include "CCSmartPtr.hpp"

CCUPtr::CCUPtr(void* ptr) : p(ptr), count(1) { }
CCUPtr::~CCUPtr() { 
	if(p) 
		delete p; 
	p = nullptr;
}

CCPtrPool* globalPool = nullptr;

CCPtrPool* CCPtrPool::GetStaticPool()
{
	return globalPool;
}
void CCPtrPool::InitPool()
{
	globalPool = new CCPtrPool();
	globalPool->pool[0] = new CCUPtr(nullptr);
}
void CCPtrPool::ReleasePool()
{
	if (globalPool) {
		globalPool->ClearUnUsedPtr();
		delete globalPool;
		globalPool = nullptr;
	}
}

CCUPtr* CCPtrPool::GetPtr(void* ptr)
{
	if (pool.find(ptr) != pool.end())
		return pool[ptr];
	return nullptr;
}
CCUPtr* CCPtrPool::AddPtr(void* ptr)
{
	CCUPtr* uptr = GetPtr(ptr);
	if (uptr) uptr->count++;
	else {
		uptr = new CCUPtr(ptr);
		pool[ptr] = uptr;
	}
	return uptr;
}
CCUPtr* CCPtrPool::AddRefPtr(void* ptr)
{
	CCUPtr* uptr = GetPtr(ptr);
	if (uptr) uptr->count++;
	return uptr;
}
CCUPtr* CCPtrPool::RemoveRefPtr(void* ptr)
{
	CCUPtr* uptr = GetPtr(ptr);
	if (uptr) {
		uptr->count--;
		if (uptr->count == 0) {
			delete uptr;
			pool.erase(ptr);
		}
	}
	return uptr;
}
void CCPtrPool::ReleasePtr(void* ptr)
{
	CCUPtr* uptr = GetPtr(ptr);
	if (uptr) {
		delete uptr;
		pool.erase(ptr);
	}
}
void CCPtrPool::ClearUnUsedPtr()
{
	for (std::unordered_map<void*, CCUPtr*>::iterator it = pool.begin(); it != pool.end(); ++it) 
		delete (*it).second;
	pool.clear();
}
