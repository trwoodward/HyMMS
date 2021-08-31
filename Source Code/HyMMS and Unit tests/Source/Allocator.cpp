#include "stdafx.h"
#include "Allocator.h"


Allocator::Allocator(std::size_t initSize)
{
	lowerBoundPtr = std::malloc(initSize);
	basePtr = topPtr = lowerBoundPtr;
	upperBoundPtr = (void*)(static_cast<char*>(lowerBoundPtr) + initSize);
}

void* Allocator::allocate(std::size_t reqSize, bool pinned)
{
	// Check if we're allocating a large object
	if (pinned)
	{
		return std::malloc(reqSize);
	}

	if (static_cast<char*>(topPtr) + reqSize > static_cast<char*>(upperBoundPtr))
	{
		//Handle failed allocation
		return NULL;
	}
	void* temp = topPtr;
	topPtr = (void*)(static_cast<char*>(topPtr) + reqSize);
	return temp;
}

void* Allocator::allocateAligned(std::size_t reqSize, std::size_t alignment, bool pinned)
{

	//Calculate the alignment offset
	//Bump topPtr by that amount (if possible)
	//Call allocate as normal.

	//Check if we are allocating a large object.
	if (reqSize >= largeObjThreshold || pinned)
	{
		// Let std::malloc handle alignment
		// We're not allocating on the heap so don't change topPtr.
		return allocate(reqSize, true);
	}

	std::size_t mask = alignment - 1;
	std::size_t misalignment = ((unsigned int)topPtr & mask);
	std::size_t adjustment = (misalignment == 0)? 0 : (alignment - misalignment);
	char* newTopPtr = static_cast<char*>(topPtr) + adjustment;
	if (newTopPtr > static_cast<char*>(upperBoundPtr))
	{
		//Handle failed allocation
		return NULL;
	}
	topPtr = (void*)newTopPtr;
	return allocate(reqSize, false);
}

Allocator::~Allocator(void)
{
	std::free(lowerBoundPtr);
}

void Allocator::setTopPtr(void* ptr)
{
	char* temp = static_cast<char*>(ptr);
	if (temp < static_cast<char*>(upperBoundPtr) 
		&& temp >= static_cast<char*>(basePtr))
	{
		topPtr = ptr;
	}
}
