#ifndef _MANAGED_ARRAY_H_
#pragma once


#include "Managed_Object.h"
#include <vector>

template<typename T>
class Managed_Array :
	public Managed_Object
{
public:
	Managed_Array<T>(int count);
	~Managed_Array<T>(void);
	int Size() { return length; }

private:
	//Used to facilitate tracing of the element objects
	std::vector<unsigned int> indices;

	int length;

	//Used for indexing
	unsigned int elementSize;
	int breakAt;

	//Used during compaction of arrays to handle nested arrays
	void* &nextArray();

	friend class Compactor;
	friend class Tracer;
	friend class GC;
	template<typename T>
	friend class SmartHandle;
};

//Used during compaction of arrays to handle nested arrays
template<typename T>
void* &Managed_Array<T>::nextArray()
{
	return pointerStoreA;
}


template<typename T>
Managed_Array<T>::Managed_Array(int count)
{
	if (count <= 0)
		throw std::exception("Array length must be > 0");

	// Check all objects are Managed_Objects
	static_cast<Managed_Object*>((T*)0);

	indices = std::vector<unsigned int>();
	breakAt = -1;

	// Allocate count x T's objects
	for (int i = 0; i < count; i++)
	{
		// In order be used with Managed_Array, must have default cstr
		// and allow assignment
		Managed_Object* temp = gcnew(T)();
		indices.push_back(temp->index);
	}
	length = count; 

	//Correctly handles large object elements
	if (sizeof(T) > GC::largeObjThreshold)
		elementSize = (unsigned int)sizeof(Managed_Object);
	else
		elementSize = (unsigned int)sizeof(T);

	GC::SetArrayFlag(index);
}

template<typename T>
Managed_Array<T>::~Managed_Array(void)
{
}


#endif

