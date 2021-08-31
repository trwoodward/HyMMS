#include "stdafx.h"
#include "Managed_Object.h"
#include "GC.h"
#include "SmartHandle.h"
#include "GCUtil.h"

Managed_Object::Managed_Object(void)
{
	isProxy = false;
	init();

	if (GC::lastObjPinned)
	{
		// Create proxy object on the heap
		Managed_Object* proxy = gcnew(Managed_Object)(this);
		index = proxy->index;

		//Add this to pinned object list
		prevObj = GC::AddToPinnedList(this);
	}
	else
	{
		//Are we on the heap?
		if (!GC::IsOnHeap(this))
			return;
		//We're on the heap
		reg();
	}
}

Managed_Object::~Managed_Object(void)
{
	if (isProxy)
	{
		GC::RemoveFromPinnedList((Managed_Object*)proxiedObj(), (Managed_Object*)nextProxiedObj());
		delete proxiedObj();
	}
}

void* Managed_Object::operator new( std::size_t objSize, std::size_t objAlignment, bool pinned)
{
	if (objSize > UINT_MAX)
		throw std::exception("Object is too large");
	if (objAlignment > CHAR_MAX)
		throw std::exception("Object alignment requirements are out of range");
	void* prevTop = GC::GetTopHeapPtr();
	void* retPtr = GC::AllocateAligned(objSize, objAlignment, pinned);
	GC::lastObjPinned = pinned;
	if(!retPtr)
	{
		GC::CollectAll();
		retPtr = GC::AllocateAligned(objSize, objAlignment, pinned);
		if (!retPtr)
			throw std::bad_alloc();
	}

	//Cache object size and alignment to store in the new object
	GC::lastAllocSize = objSize;
	GC::lastAlignment = objAlignment;

	//If not first object on the heap, store the padding from this object
	if (GC::LastAllocIndx != INDEX_SENTINEL)
	{
		//We only care about the last byte of the address, alignment requirement shouldn't be higher than 255 bytes.
		((Managed_Object*)GC::Get(GC::LastAllocIndx))->nextObjPadding = CalculateAdjustment(prevTop, objAlignment);
	}
	return retPtr;
}

void* Managed_Object::operator new(std::size_t objSize, void* ptr)
{
	if (!ptr)
		throw std::exception("Cannot create object at address NULL");
	return ptr;
}

void Managed_Object::operator delete( void*, std::size_t )
{
	//No need to free memory
}

void Managed_Object::operator delete(void* ptr, std::size_t sze, bool pinned)
{
	//No need to free memory
}

void Managed_Object::operator delete( void* ptr, void* place )
{
	//No need to free memory
}

Managed_Object& Managed_Object::operator=( Managed_Object const& other )
{
	//Created to prevent member-wise copying
	//Base object relates to 'memory slot' so shouldn't change on copying
	return *this;
}

Managed_Object::Managed_Object( Managed_Object* toProxy )
{
	init();
	proxiedObj() = toProxy;
	reg();
}

inline void Managed_Object::init()
{
	index = INDEX_SENTINEL;
	objSize = GC::lastAllocSize;
	objAlignment = GC::lastAlignment;
	GC::lastAlignment = 1;
	nextObjPadding = 0;
	prevObj = NULL;
	lastHandleOffset() = firstHandleOffset() = NULL;
}

inline void Managed_Object::reg()
{
	index = GC::Add(this);
	if (GC::LastAllocIndx != INDEX_SENTINEL)
		prevObj = (Managed_Object*)(GC::Get(GC::LastAllocIndx));
	GC::LastAllocIndx = index;
}

void Managed_Object::removeNurseryFlags()
{
	Managed_Object* obj = (isProxy) ? (Managed_Object*)proxiedObj() : this;
	if (!obj)
		throw std::exception("Object is proxy but proxyedObj pointer is NULL");
	if (obj->firstHandleOffset() == NULL || GC::isArray(obj->getIndex()))
		return;
	SmartHandle<Managed_Object>* currHndl = (SmartHandle<Managed_Object>*)((char*)obj + (unsigned int)obj->firstHandleOffset());
	while (true)
	{
		currHndl->UnsetNurseryFlag();
		void* nextOffset = currHndl->getNextOffset();
		if (nextOffset == NULL)
		{
			return;
		}
		currHndl = (SmartHandle<Managed_Object>*)((char*)obj + (unsigned int)nextOffset);
	}
}

bool Managed_Object::isInBounds( void* addr )
{
	if (!addr)
		return false;
	return (!((char*)addr < (char*)this || (char*)addr >= getUpperBound()));
}

unsigned int Managed_Object::getIndex()
{
	return index;
}

unsigned int Managed_Object::getSize()
{
	return objSize;
}

char Managed_Object::getNextObjPadding()
{
	return nextObjPadding;
}

void* Managed_Object::getUpperBound()
{
	return (void*)((char*)this + objSize);
}

void* & Managed_Object::firstHandleOffset()
{
	return pointerStoreA;
}

void* & Managed_Object::lastHandleOffset()
{
	return pointerStoreB;
}

void* & Managed_Object::proxiedObj()
{
	return pointerStoreA;
}

void* & Managed_Object::nextProxiedObj()
{
	return pointerStoreB;
}


