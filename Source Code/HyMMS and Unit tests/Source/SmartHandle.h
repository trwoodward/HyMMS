#pragma once
#ifndef  _SMARTHANDLE_H_
#define _SMARTHANDLE_H_
#include "GC.h"
#include "Managed_Object.h"
#include "GCUtil.h"
#include "Compactor.h"
#include "Handle_Array.h"

#include <windows.h>
#include <sstream>

#define ROOT_PTR 1
#define IN_NURSERY 2

// Version 4: Implements simple indirection via a table of pointers
// overrides * and -> operators so that it can be used in place of
// conventional pointers.
// Defers registration of the object with the reference table to the
// base class of the object itself.
// Uses static_cast<> to cast the pointer to the object to a pointer
// to a Managed_Object to enforce inheritance from Managed_Object at
// compile time.
// Automatically detects on creation whether it lies inside the most recently constructed Managed_Object.
// If it does then it registers itself with that Managed_Object.
// The registration is deferred until such time as the handle is pointed at
// A Managed_Object. Note - this needs to be checked in the context of 
// Initialization lists.
template<typename T>
class SmartHandle
{
public:
	//Allows for creation of uninitialized SmartHandles
	SmartHandle<T>(void);

	//Initializing constructor
	SmartHandle<T>(T*);

	//Copy constructor
	SmartHandle<T>(const SmartHandle<T>&);

	//Array constructor
	SmartHandle<T>(Managed_Array<T>*);

	//Constructor for setting to NULL
	SmartHandle<T>(int nullVal);

	~SmartHandle<T>(void);

	//De-referencing operators
	T& operator*();
	const T& operator*() const; //for calling from const functions

	//Arrow operators
	T* operator->();
	const T* operator->() const; //for calling from const functions

	T& operator[](const int reqIndex);

	//Assignment operators
	SmartHandle<T>& operator=(const SmartHandle<T>&);
	SmartHandle<T>& operator=(const T* &);

	//bool operator!=(const SmartHandle<T>&);
	//bool operator==(const SmartHandle<T>&);

protected:
	//Used by constructors to initialize and register handle
	void init();

	//Write barrier
	void WriteBarrier(unsigned int oldIndex);

	//inserts handle into Managed_Object's linked list
	void insertSelfIntoList(void* objAddr, void* &lastHndlOffsetRef);

	//Setters for HandleFlags
	void UnsetNurseryFlag();
	void UnsetRootFlag();
	void SetNurseryFlag();
	void SetRootFlag();

	//Getter for offset of next Handle in linked handle list
	void* getNextOffset();

	//index into reference table
	unsigned int index;

	//Absolute offset from owning object base addr to next handle
	void* nextHndl;

	//Absolute offset from owning object bass addr to prev handle
	void* prevHndl;

	//Used to indicate that this is a root pointer
	//Or that the SmartHandle is a member of a nursery object
	char HandleFlags;

	friend class GC;
	friend class Managed_Object;
	friend class Tracer;
	template<typename U>
	friend bool operator!=(const SmartHandle<U>&, const int&);
	template<typename U>
	friend bool operator==(const SmartHandle<U> &, const int&);
	template<typename U, typename V>
	friend bool operator!=(const SmartHandle<U>&, const SmartHandle<V>&);
	template<typename U, typename V>
	friend bool operator==(SmartHandle<U> const&, SmartHandle<V> const&);

	//Remove in release version
	friend class TestSpy;
};

//Default constructor
template<typename T>
SmartHandle<T>::SmartHandle()
{
	index = INDEX_SENTINEL;
	init();
}

//Copy constructor
template<typename T>
SmartHandle<T>::SmartHandle(const SmartHandle<T>& other)
{
	index = other.index;
	//Call init in case being called from initialization list
	init();
	WriteBarrier(INDEX_SENTINEL);
}

//Construct from Managed_Object pointer
template<typename T>
SmartHandle<T>::SmartHandle(T* p)
{
	if (!p)
	{
		index = INDEX_SENTINEL;
		return;
	}
	//static_cast used to check inheritance from Managed_Object at compile time
	Managed_Object* m = static_cast<Managed_Object*>(p);
	index = m->getIndex();
	//Call init in case being called from initialization list
	init();
	WriteBarrier(INDEX_SENTINEL);
}

//Construct from Managed_Array pointer
template<typename T>
SmartHandle<T>::SmartHandle(Managed_Array<T>* arr)
{
	if (!arr)
	{
		index = INDEX_SENTINEL;
		return;
	}
	index = arr->getIndex();
	init();
	WriteBarrier(INDEX_SENTINEL);
}

//Construct from int to enable setting to Null
template<typename T>
SmartHandle<T>::SmartHandle(int nullval)
{
	if (nullval != NULL)
		throw std::exception("Cannot initialize SmartHandle with integer");
	index = INDEX_SENTINEL;
	init();
}

template<typename T>
SmartHandle<T>::~SmartHandle(void)
{
	if (HandleFlags & ROOT_PTR)
	{
		GC::DeregisterRootHandle(this);
	}
	if (index != INDEX_SENTINEL)
	{
		if (GC::isColour(index, GC_GREEN_FLAG))
		{
			if (!(HandleFlags & IN_NURSERY))
				GC::DeregisterNurseryRoot(index);
		}
	}
}

template<typename T>
T& SmartHandle<T>::operator*() 
{
	T* obj = (T*)GC::Get(index);
	if (!obj)
		throw std::exception("Attempted to de-reference NULL");
	if (!obj->isProxy)
		return *obj;
	else
	{
		return *((T*)obj->proxiedObj());
	}
}

template<typename T>
const T& SmartHandle<T>::operator*() const
{
	T* obj = (T*)GC::Get(index);
	if (!obj)
		throw std::exception("Attempted to de-reference NULL");
	if (!obj->isProxy)
		return *obj;
	else
	{
		return *((T*)obj->proxiedObj());
	}
}

template<typename T>
T* SmartHandle<T>::operator->()
{
	T* obj = (T*)GC::Get(index);
	if (!obj)
		throw std::exception("Attempted to de-reference NULL");
	if (!obj->isProxy)
		return obj;
	else
	{
		return (T*)obj->proxiedObj();
	}
}

template<typename T>
const T* SmartHandle<T>::operator->() const
{
	T* obj = (T*)GC::Get(index);
	if (!obj)
		throw std::exception("Attempted to de-reference NULL");
	if (!obj->isProxy)
		return obj;
	else
	{
		return (T*)obj->proxiedObj();
	}
}

template<typename T>
T& SmartHandle<T>::operator[]( const int reqIndex )
{
	if (index != INDEX_SENTINEL)
	{
		Managed_Object* temp = (Managed_Object*)GC::Get(index);
		if (!temp)
			throw std::exception("Cannot use subscript operator on uninitialized handle");
		if (GC::isArray(temp->getIndex()))
		{
			// Can safely cast up to Managed_Array
			Managed_Array<T>* arr = (Managed_Array<T>*)temp;

			unsigned int compactorOffset = 0;

			if (arr->breakAt >= 0)
			{
				if (reqIndex >= arr->breakAt)
					compactorOffset = GC::compactor->incHeapCumulative;
			}

			// Locate desired object
			temp = (Managed_Object*)((char*)arr + arr->objSize + arr->nextObjPadding + (reqIndex * arr->elementSize)) + compactorOffset;
			if (temp->isProxy)
			{
				// return ref to proxyed object
				return *((T*)temp->proxiedObj());
			}
			else
			{
				return *((T*)temp);
			}
		}
	}
	// SmartHandle doesn't point to a valid array object
	throw std::exception("SmartHandle doesn't reference a valid array object.");
}

//Assignment operator
template<typename T>
SmartHandle<T>& SmartHandle<T>::operator=(const SmartHandle<T>& other)
{
	int oldIndex = index;
	index = other.index;
	WriteBarrier(oldIndex);
	return *this;
}

//Assignment from Managed_Object pointer
template<typename T>
SmartHandle<T>& SmartHandle<T>::operator=(const T* & p)
{
	int oldIndex = index;
	if (!p)
	{
		index = INDEX_SENTINEL;
		return NULL;
	}
	//static_cast used to check inheritance from Managed_Object at compile time
	Managed_Object* m = static_cast<Managed_Object*>(p);
	index = m->getIndex();
	WriteBarrier(oldIndex);
	return *this;
}

template<typename T>
void SmartHandle<T>::init() 
{
	nextHndl = prevHndl = NULL;
	HandleFlags = 0;
	if (GC::IsOnHeap(this))
	{
		Managed_Object* currObj = GC::GetLastAllocObj();
		if (!currObj)
			throw std::exception("SmartHandle is on the managed heap despite there being no Managed_Objects on the managed heap");
		while (!(currObj->isInBounds(this)))
		{
			if (currObj->prevObj == NULL)
				throw std::exception("Start of heap reached without containing Managed_Object being found, SmartHandle.init");
			else
				currObj = currObj->prevObj;
		}

		//CurrObj now points to the object to be registered with.
		UnsetRootFlag();
		currObj->RegisterHandle(this);
		if (GC::isColour(currObj->getIndex(), GC_GREEN_FLAG))
			SetNurseryFlag();
		else
			UnsetNurseryFlag();
	}
	else
	{
		//Check if within the bounds of the pinned objects
		if (GC::firstPinnedObj)
		{
			if (GC::MightBePinned(this))
			{
				// Traverse the pinned object list and look for an object that it lies within.
				Managed_Object* currPinned = (Managed_Object*)GC::firstPinnedObj;
				while (!currPinned->isInBounds(this))
				{
					if (currPinned->prevObj == NULL)
					{
						// Pinned obj list fully traversed, this isn't a member of any of them
						SetRootFlag();
						GC::RegisterRootHandle(this);
						return;
					}
					else
					{
						currPinned = currPinned->prevObj;
					}
				}
				//currPinned points to the object to be registered with.
				UnsetRootFlag();
				currPinned->RegisterHandle(this);
				if (GC::isColour(currPinned->index, GC_GREEN_FLAG))
					SetNurseryFlag();
				else
					UnsetNurseryFlag();
				return;
			}
		}

		//Register as root pointer
		SetRootFlag();
		GC::RegisterRootHandle(this);
	}
}

template<typename T>
void SmartHandle<T>::WriteBarrier(unsigned int oldIndex)
{
	bool green = GC::isColour(index, GC_GREEN_FLAG);
	if (GC::isColour(index, GC_WHITE_FLAG) && !green)
		GC::SetColour(index, GC_GREY_FLAG);
	if (GC::isColour(oldIndex, GC_GREEN_FLAG))
	{
		if (index != oldIndex)
			GC::DeregisterNurseryRoot(oldIndex);
	}
	if (green && !(HandleFlags & IN_NURSERY))
	{
		GC::RegisterNurseryRoot(index);
	}
}

//Adds handle to given linked list
template<typename T>
void SmartHandle<T>::insertSelfIntoList(void* objAddr, void* &lastHndlOffsetRef)
{
	if (!objAddr)
		throw std::exception("Cannot insert handle into the list of a NULL object");
	prevHndl = lastHndlOffsetRef;
	SmartHandle<T>* lastHandle = (SmartHandle<T>*)((char*)objAddr + (unsigned int)lastHndlOffsetRef);
	lastHandle->nextHndl = (void*)((char*)this - (char*)objAddr);
	lastHndlOffsetRef = lastHandle->nextHndl;
}

template<typename T>
inline void SmartHandle<T>::UnsetNurseryFlag()
{
	HandleFlags = HandleFlags & (~IN_NURSERY);
}

template<typename T>
inline void SmartHandle<T>::UnsetRootFlag()
{
	HandleFlags = HandleFlags & (~ROOT_PTR);
}

template<typename T>
inline void SmartHandle<T>::SetNurseryFlag()
{
	HandleFlags = HandleFlags | IN_NURSERY;
}

template<typename T>
inline void SmartHandle<T>::SetRootFlag()
{
	HandleFlags = HandleFlags | ROOT_PTR;
}

template<typename T>
inline void* SmartHandle<T>::getNextOffset()
{
	return nextHndl;
}

template<typename U>
bool operator!=(SmartHandle<U> const& hndl, int const& isNull)
{
	return (hndl.index != INDEX_SENTINEL);
}

template<typename U>
bool operator==(SmartHandle<U> const& hndl, int const& isNull)
{
	return (hndl.index == INDEX_SENTINEL);
}

template<typename U, typename V>
bool operator==(SmartHandle<U> const& first, SmartHandle<V> const& second)
{
	return (first.index == second.index);
}

template<typename U, typename V>
bool operator!=(SmartHandle<U> const& first, SmartHandle<V> const& second)
{
	return (first.index != second.index);
}

#endif // ! _SMARTHANDLE_H_