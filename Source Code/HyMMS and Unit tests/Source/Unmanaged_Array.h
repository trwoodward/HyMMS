#ifndef  _UNMANAGED_ARRAY_H_
#pragma once

#include "Managed_Object.h"

template<typename T>
class Unmanaged_Array :
	public Managed_Object
{
public:
	Unmanaged_Array<T>(int count);
	~Unmanaged_Array<T>(void);

	int Size() { return length; }

private:
	int length;

	//Needed for element access
	char firstObjPadding;

	friend class Compactor;
	friend class Tracer;
	friend class GC;
	template<typename T>
	friend class SmartHandle;
	template<typename T>
	friend class UnmanagedArrHandle;
};

template<typename T>
Unmanaged_Array<T>::Unmanaged_Array(int count)
{
	if (count < 0)
		throw std::exception("Array length may not be negative");

	if (count == 0) // array is being proxyed, proxy will set up internal members
		return;

	if (__alignof(T) > CHAR_MAX)
		throw std::exception("Error: alignment requirements of array element are out of range");

	firstObjPadding = CalculateAdjustment((void*)sizeof(Unmanaged_Array<T>), (char)(__alignof(T)));
	objSize = objSize + (count * sizeof(T)) + firstObjPadding;	

	if (objSize > GC::largeObjThreshold)
	{
		//Because malloc automatically returns a block correctly aligned
		//for all types found by the compiler, adjustment will be the same
		//as if malloc had returned 0

		void* block = std::malloc(sizeof(Unmanaged_Array<T>) + (count * sizeof(T)) + firstObjPadding);
		isProxy = true;
		proxiedObj() = block;
		Unmanaged_Array<T>* temp = new(block) Unmanaged_Array<T>(0);
		temp->index = index;
		temp->firstObjPadding = firstObjPadding;

		//Set up proxyed object
		temp->objSize = sizeof(Unmanaged_Array<T>) + (count * sizeof(T));
		temp->length = count;
		temp->prevObj = GC::AddToPinnedList(temp);
		
		// Set up data members
		T* elementTemp = (T*)((char*)block + sizeof(Unmanaged_Array<T>) + firstObjPadding);
		for (int i = 0; i < count; ++i)
		{
			elementTemp = new(elementTemp) T();
			elementTemp = (T*)((char*)elementTemp + sizeof(T));
		}
		objSize = sizeof(Unmanaged_Array<T>);
	}
	else
	{
		//Alignment handled here
		void* block = GC::AllocateAligned(count * sizeof(T), __alignof(T), false);
		if (!block)
		{
			GC::CollectAll();
			block = GC::AllocateAligned(count * sizeof(T), __alignof(T), false);
			if (!block)
				throw std::bad_alloc();
		}
		GC::lastAllocSize = objSize;
		for (int i = 0; i < count; i++)
		{
			// alloc the SmartHandles on the heap
			// use placement new to create the SmartHandles on the heap
			new(block) T();
			block = (void*)((char*)block + sizeof(T));
		}
		length = count;
	}
}

template<typename T>
Unmanaged_Array<T>::~Unmanaged_Array()
{

}


#endif // ! _UNMANAGED_ARRAY_H_