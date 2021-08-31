#pragma once
#include "GCTimer.h"
#include "Managed_Object.h"
#include "Managed_Array.h"

//Compactor should properly belong to the allocator since it has
//responsibility for controlling the heap. Can also be accessed indirectly
//through the general GC interface.
class Compactor
{
public:
	Compactor(void);

	//Compact arbitrary range of objects
	void* Compact(void*, void*, void*&);

	//Atomically compact Nursery
	void* NurseryCompact(void* oldGenEnd, void* heapEnd, void*& firstObject);

	//Compact until budget expires
	void* IncrementalCompact(void*&, double msBudget);

	//Used to keep track of changing heap during incremental compaction
	void SetIncHeapStart(void*);
	void SetIncHeapEnd(void*);

	~Compactor(void);

private:
	//Helper functions shared between different compactor methods
	void* HandleLiveObject(void* &cursor, Managed_Object* &prevLiveObj, std::size_t& cumulative, void* &firstObject, void* &newTopPtr); 
	void HandleDeadObject(void* &cursor, std::size_t& cumulative, void* prevLiveObj, void* firstObj, void* heapStart);

	//Used to deal with compacting Managed_Arrays
	Managed_Array<Managed_Object>* currArr;
	int currArrayIndx;
	int arrLength;

	//Used to hold state information between compaction increments
	void* incHeapStart;
	void* incHeapEnd;
	void* incHeapCursor;
	void* incNewTopPtr;
	Managed_Object* incPrevLiveObj;
	std::size_t incHeapCumulative;
	
	template<typename T>
	friend class SmartHandle;
};

