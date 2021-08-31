#pragma once
#include <cstdlib>

class Allocator
{
public:
	Allocator(std::size_t);

	//Allocates object ignoring alignment requirements, handles pinned object requests
	void* allocate(std::size_t, bool);

	//Allocates block large enough for object + alignment requirements
	void* allocateAligned(std::size_t, std::size_t, bool);

	//Returns heap memory to operating system
	~Allocator(void);

private:
	//Used to reset top of heap after compaction
	void setTopPtr(void*);

	//Markers for heap & occupied heap
	void* basePtr;
	void* topPtr;
	void* upperBoundPtr;
	void* lowerBoundPtr;

	std::size_t largeObjThreshold;

	friend class GC;
	friend class Tracer;

	//For testing purposes only
	friend class TestSpy;
};

