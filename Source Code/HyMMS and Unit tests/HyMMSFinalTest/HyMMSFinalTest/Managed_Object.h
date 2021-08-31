#pragma once
#include <iostream>

class Managed_Object
{
public:
	Managed_Object(void);
	virtual ~Managed_Object(void);

	void* operator new(std::size_t, std::size_t, bool);
	void* operator new(std::size_t, void*);

	void operator delete(void*, std::size_t);
	void operator delete(void* ptr, std::size_t sze, bool pinned);
	void operator delete(void* ptr, void* place);

	Managed_Object& operator=(Managed_Object const& other);
	
private:
	//friend declarations
	template<typename T>
	friend class SmartHandle;
	friend class Compactor;
	friend class GC;
	friend class Tracer;
	template <typename T>
	friend class Managed_Array;
	template <typename T>
	friend class Handle_Array;
	template <typename T>
	friend class HndlArrHandle;
	template <typename T>
	friend class Unmanaged_Array;
	template <typename T>
	friend class UnmanagedArrHandle;

	//Internal initialization
	Managed_Object(Managed_Object* toProxy);
	void init();
	void reg();

	//Handle registration
	template<typename T>
	void RegisterHandle(SmartHandle<T>* hndl);

	//Helper methods
	void removeNurseryFlags();
	bool isInBounds(void* addr);

	//Accessor methods
	unsigned int getIndex();
	unsigned int getSize();
	char getNextObjPadding();
	void* getUpperBound();
	void* &firstHandleOffset();
	void* &lastHandleOffset();
	void* &proxiedObj();
	void* &nextProxiedObj();

	//Variables

	//Internal object state
	unsigned int index;
	std::size_t objSize;
	char objAlignment;
	bool isProxy;

	//Heap linked list variables
	char nextObjPadding;
	Managed_Object* prevObj;
	void* pointerStoreA;
	void* pointerStoreB;

	//Remove in release version
	friend class TestSpy;
};

template<typename T>
void Managed_Object::RegisterHandle( SmartHandle<T>* hndl )
{
	if (!hndl)
		throw std::exception("Cannot register NULL pointer as Handle");
	if (firstHandleOffset() == NULL)
	{
		firstHandleOffset() = (void*)((char*)hndl - (char*)this);
		lastHandleOffset() = firstHandleOffset();
	}
	else
	{
		hndl->insertSelfIntoList(this, lastHandleOffset());
	}
}

