//
// Created by roger on 2020/10/30.
//

#ifndef VR720_CCSMARTPTR_HPP
#define VR720_CCSMARTPTR_HPP

#include <unordered_map>

//ģ������Ϊ��ԪʱҪ��������
template <typename T>
class CCSmartPtr;

//������
class CCUPtr
{
public:

    //���캯���Ĳ���Ϊ���������ָ��
    CCUPtr(void* ptr);

    //��������
    ~CCUPtr();

    //���ü���
    int count;

    //��������ָ��
    void *p;
};

//ָ���
class CCPtrPool {

public:

    std::unordered_map<void*, CCUPtr*> pool;

    static CCPtrPool* GetStaticPool();
    static void InitPool();
    static void ReleasePool();

    CCUPtr* GetPtr(void* ptr);
    CCUPtr* AddPtr(void* ptr);
    CCUPtr* AddRefPtr(void* ptr);
    CCUPtr* RemoveRefPtr(void* ptr);
    void ReleasePtr(void* ptr);
    void ClearUnUsedPtr();
};
#define CCPtrPoolStatic CCPtrPool::GetStaticPool()

//����ָ����
template <typename T>
class CCSmartPtr
{
private:
    T* ptr = nullptr;
    CCUPtr* rp = nullptr;  //���������ָ��
public:
    CCSmartPtr() {  
        ptr = nullptr;
        rp = CCPtrPoolStatic->AddPtr(nullptr);   
    }
    //���캯��
    CCSmartPtr(T *srcPtr)  { 
        ptr = srcPtr;
        rp = CCPtrPoolStatic->AddPtr(srcPtr);
    }      
    //���ƹ��캯��
    CCSmartPtr(const CCSmartPtr<T> &sp)  {      
        ptr = sp.ptr;
        rp = CCPtrPoolStatic->AddRefPtr(sp.ptr);
    }     

    //���ظ�ֵ������
    CCSmartPtr& operator = (const CCSmartPtr<T>& rhs) {    
        CCPtrPoolStatic->RemoveRefPtr(ptr);
        ptr = rhs.ptr;
        rp = CCPtrPoolStatic->AddRefPtr(ptr);
        return *this;
    }

    T & operator *() const { //����*������
        return *ptr;
    }
    T* operator ->() const { //����->������
        return ptr;
    }

    bool IsNullptr() const {
        return ptr == nullptr;
    }
    T* GetPtr() const { return ptr;  }
    int CheckRef() {
        if (rp) return rp->count;
        return 0;
    }
    int AddRef() {
        if (rp) 
            return ++rp->count;
        return 0;
    }
    void ForceRelease() {
        CCPtrPool::GetStaticPool()->ReleasePtr(ptr);
        ptr = nullptr;
        *rp = nullptr;
    }

    ~CCSmartPtr() {        //��������
        CCPtrPoolStatic->RemoveRefPtr(ptr);
        ptr = nullptr;
        rp = nullptr;
    }

};


#endif //VR720_CCSMARTPTR_HPP
