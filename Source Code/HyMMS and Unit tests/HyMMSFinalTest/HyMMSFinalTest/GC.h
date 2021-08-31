#pragma once
#include <vector>
#include "Allocator.h"
#include "SmartHandle.h"
#include "GCTimer.h"


#define GC_NURSERY_COLLECT_NEXT 1
#define GC_TRACING 2
#define GC_COMPACTING 4

//Creates and provides access to the reference table
//through which SmartHandles access the underlying objects
class GC
{
public:
	static void CreateHeap(std::size_t);
	static void DestroyHeap();
	static void SetCollectorBudget(double);
	static void SetLargeObjThreshold(std::size_t threshold);
	static void IncCollect();
	static void CollectAll();

	//For demo only
	static double GetLastNurCollectionTime();
	static double GetLastFullCollectionTime();
	static int GetNumCols();

private:	
	//For demo only
	static double lastColStamp;
	static double lastColDur;
	static double lastNurDur;
	static int numCols;

	//friend declarations
	friend class Managed_Object;
	friend class Compactor;
	friend class Tracer;
	template <typename T>
	friend class SmartHandle;
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

	//Methods

	//Allocation method called by Managed_Object::operator new()
	static void* AllocateAligned(std::size_t, std::size_t, bool);

	//Internal collector methods
	static void CompactHeap();
	static void IncCompactHeap(double);
	static void TraceFullHeap();
	static void FullOldGenCollection();
	static void NurseryCollect();
	static void FinalizeAllObjects();

	//Registration of roots
	template <typename T>
	static void RegisterRootHandle(SmartHandle<T>*);
	template <typename T>
	static void DeregisterRootHandle(SmartHandle<T>*);

	//Registration of Nursery roots
	static void RegisterNurseryRoot(unsigned int index);
	static void DeregisterNurseryRoot(unsigned int index);
	static void ClearNurseryRoots();

	//Reference table management
	static unsigned int Add(void *);
	static void* Get(unsigned int);
	static void RemoveRefAt(unsigned int);
	static void UpdateRef(unsigned int, void *);

	//Flag handling
	static bool isColour(unsigned int index, unsigned int colour);
	static bool isArray(unsigned int index);
	static bool isFlagSet(unsigned int index, unsigned int flag);
	static void SetColour(unsigned int index, unsigned int colour);
	static void SetArrayFlag(unsigned int index);
	static void SetHandleArrayFlag(unsigned int index);
	static void MarkAllWhite();

	//Pinned object management
	static Managed_Object* AddToPinnedList(Managed_Object* pinnedObj);
	static void RemoveFromPinnedList(Managed_Object* pinnedObj, Managed_Object* nextPinnedObj);

	//Accessor methods
	static void* GetTopHeapPtr();
	static void* GetBaseHeapPtr();
	static Managed_Object* GetLastAllocObj();

	//Helper methods
	static bool IsOnHeap(void* addr);
	static bool MightBePinned(void* addr);

	//Reference table entry structure
	struct RefEntry 
	{
		void* ptr;
		char Flags;
	};

	//Variables

	//Components
	static std::vector<RefEntry>* refTable;
	static std::vector<unsigned int> nurseryRoots;
	static Allocator* allocator;
	static Compactor* compactor;
	static Tracer* tracer;

	// Last object to be allocated
	static unsigned int LastAllocIndx;
	// Last object to be promoted (promotion isn't final until 
	// incremental compactor has compacted the objects).
	static unsigned int LastPromotedIndx;

	//Collector state variables
	static double collectorBudget;
	static char collectorFlags;
	static char nurCollFreq;
	static char lastNurColl;
	static std::size_t largeObjThreshold;

	//Heap state variables
	static void* nurGenPtr;
	static void* promoBuffEndPtr;

	//Last allocated object state variables
	static std::size_t lastAllocSize;
	static char lastAlignment;
	static bool lastObjPinned;

	//Root handle list variables
	static void* firstHandle;
	static void* lastHandle;

	//Pinned object list variables
	static void* firstPinnedObj;
	static void* lowestPinnedAddr;
	static void* highestPinnedAddr;

	//Remove in release version
	friend class TestSpy;
};

template <typename T>
void GC::DeregisterRootHandle(SmartHandle<T>* toDereg)
{
	if (!toDereg)
		return;
	if ((firstHandle == NULL) || (lastHandle == NULL))
	{
		//We have no root handles to de-register
		return;
	}
	//Compute pointers to the pointers that should be pointing to this object if its in the root handle linked list
	void** prevRef = NULL;
	void** nextRef = NULL;
	if (toDereg->prevHndl == NULL)
	{
		//If previous handle is NULL must be pointed to by firstHandle
		prevRef = &firstHandle;
	}
	else
	{
		//otherwise must be pointed to by previous object's next handle pointer
		prevRef = &(((SmartHandle<Managed_Object>*)toDereg->prevHndl)->nextHndl);
	}
	if (toDereg->nextHndl == NULL)
	{
		//If next handle is NULL must be pointed to by lastHandle
		nextRef = &lastHandle;
	}
	else
	{
		//otherwise must be pointed to by next object's previous handle pointer
		nextRef = &(((SmartHandle<Managed_Object>*)toDereg->nextHndl)->prevHndl);
	}
	if (((char*)(*prevRef) == (char*)toDereg) && ((char*)(*nextRef) == (char*)toDereg))
	{
		//If the previous and next pointers point to this object it was in the linked list
		//So close up the gap in the list by pointing them at this object's prev/ next handle pointers.
		(*prevRef) = toDereg->nextHndl;
		(*nextRef) = toDereg->prevHndl;
	}
	else
	{
		//This wasn't a root handle
		return;
	}
}

template <typename T>
void GC::RegisterRootHandle(SmartHandle<T>* toReg)
{
	if (!toReg)
		return;
	if(firstHandle == NULL || lastHandle == NULL)
	{
		//First handle
		firstHandle = lastHandle = (void*)toReg;
	}
	else
	{
		//Add to root handle linked list
		SmartHandle<T>* endHandle = (SmartHandle<T>*)lastHandle;
		toReg->prevHndl = (void*)((char*)endHandle);
		endHandle->nextHndl = (void*)((char*)toReg);
		lastHandle = (void*)toReg;
	}
}

