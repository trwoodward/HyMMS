#include "stdafx.h"
#include "GC.h"
#include "Compactor.h"
#include "Tracer.h"
#include "GCUtil.h"
#include "Managed_Object.h"
#include "Managed_Array.h"
#include <vector>

//Static variable definitions
std::vector<GC::RefEntry>* GC::refTable;
Allocator* GC::allocator;
Compactor* GC::compactor;
Tracer* GC::tracer;
unsigned int GC::LastAllocIndx = INDEX_SENTINEL;
unsigned int GC::LastPromotedIndx = INDEX_SENTINEL;
void* GC::firstHandle = NULL;
void* GC::lastHandle = NULL;
char GC::collectorFlags = GC_NURSERY_COLLECT_NEXT | GC_TRACING;
char GC::nurCollFreq = 3;
char GC::lastNurColl = 0;
double GC::collectorBudget = 0.0;
void* GC::nurGenPtr = NULL;
void* GC::promoBuffEndPtr = NULL;
std::vector<unsigned int> GC::nurseryRoots;
std::size_t GC::largeObjThreshold = INDEX_SENTINEL;
std::size_t GC::lastAllocSize = 0;
char GC::lastAlignment = 1;
void* GC::firstPinnedObj = NULL;
void* GC::lowestPinnedAddr = NULL;
void* GC::highestPinnedAddr = NULL;
bool GC::lastObjPinned = false;


//For demo only
double GC::lastColStamp = 0.0;
double GC::lastColDur = 0.0;
double GC::lastNurDur = 0.0;
int GC::numCols = 0;

double GC::GetLastNurCollectionTime()
{
	return lastNurDur;
}

double GC::GetLastFullCollectionTime()
{
	return lastColDur;
}

int GC::GetNumCols()
{
	return numCols;
}

//End demo code


void GC::CreateHeap(std::size_t heapRequest)
{
	if (heapRequest == 0)
		throw std::exception("Heap size must be greater than zero");
	if (!(allocator = new Allocator(heapRequest)))
		throw std::exception("Allocator creation failed");
	if (!(compactor = new Compactor()))
		throw std::exception("Compactor creation failed");
	if (!(tracer = new Tracer()))
		throw std::exception("Tracer creation failed");
	compactor->SetIncHeapStart(allocator->basePtr);
	nurGenPtr = promoBuffEndPtr = allocator->basePtr;
	largeObjThreshold = UINT_MAX; // max offset that can be held in a 4-byte pointer
	allocator->largeObjThreshold = largeObjThreshold;
}

void GC::DestroyHeap()
{
	FinalizeAllObjects();

	if (allocator)
	{
		delete allocator;
		allocator = NULL;
	}
	if (compactor)
	{
		delete compactor;
		compactor = NULL;
	}
	if (tracer)
	{
		delete tracer;
		tracer = NULL;
	}
	if (refTable)
	{
		delete refTable;
		refTable = NULL;
	}
	//Reset all static variables
	LastAllocIndx = INDEX_SENTINEL;
	LastPromotedIndx = INDEX_SENTINEL;
	firstHandle = NULL;
	lastHandle = NULL;
	collectorBudget = 0.0;
	nurGenPtr = NULL;
	collectorFlags = 0;
	nurseryRoots.clear();
	promoBuffEndPtr = NULL;
	lastNurColl = 0;
	lastAllocSize = 0;
	lastAlignment = 1;
	lastObjPinned = false;
	firstPinnedObj = NULL;
	lowestPinnedAddr = NULL;
	highestPinnedAddr = NULL;
	nurCollFreq = 3;
	largeObjThreshold = INDEX_SENTINEL;
}

//Calls destructor of any object left on the heap
void GC::FinalizeAllObjects()
{
	if (!refTable)
		return;
	for (unsigned int i = 0; i < refTable->size(); ++i)
	{
		if ((*refTable)[i].ptr != NULL)
		{
			Managed_Object* temp = (Managed_Object*)((*refTable)[i].ptr);
			temp->~Managed_Object();
		}
	}
}

void GC::SetCollectorBudget( double budget )
{
	if (budget <= 0.0)
		throw std::exception("Collector time budget must be positive");
	collectorBudget = budget;
}

void GC::SetLargeObjThreshold( std::size_t threshold )
{
	if (threshold > UINT_MAX)
	{
		largeObjThreshold = allocator->largeObjThreshold = UINT_MAX;
	}
	else if (threshold > sizeof(Managed_Object))
	{
		largeObjThreshold = allocator->largeObjThreshold = threshold;
	}
	else
	{
		throw std::exception("Large object threshold must be > sizeof(Managed_Object)");
	}
}

void GC::IncCollect()
{
	// Nursery collections take priority
	if (collectorFlags & GC_NURSERY_COLLECT_NEXT)
	{
		//double newTime = G_GetCurrTime(); //For Demo use only
		NurseryCollect();
		//lastNurDur = G_GetCurrTime() - newTime; //For Demo use only
		//numCols++; //For Demo use only
		collectorFlags = collectorFlags & (~GC_NURSERY_COLLECT_NEXT);
		lastNurColl = 0;
		return;
	}
	if (collectorFlags & GC_TRACING)
	{
		double remaining = tracer->IncrementalTrace(collectorBudget);
		if (remaining > 0)
		{
			collectorFlags = GC_COMPACTING;
			IncCompactHeap(remaining);
		}
		else if (remaining == 0.0)
		{
			collectorFlags = GC_COMPACTING;
		}
	}
	else if (collectorFlags & GC_COMPACTING)
	{
		IncCompactHeap(collectorBudget);
	}
	++lastNurColl;
	if (lastNurColl > 1)
	{
		collectorFlags = collectorFlags | GC_NURSERY_COLLECT_NEXT;
	}
}

void GC::CollectAll()
{
	NurseryCollect();
	CompactHeap();
	FullOldGenCollection();
	nurGenPtr = allocator->topPtr;
	collectorFlags = GC_TRACING;
	lastNurColl = 0;
}

void* GC::AllocateAligned( std::size_t requestSize, std::size_t alignment, bool pinned)
{
	return allocator->allocateAligned(requestSize, alignment, pinned);
}

void GC::CompactHeap()
{
	void* newTopPtr = compactor->Compact(allocator->lowerBoundPtr, allocator->topPtr, allocator->basePtr);
	allocator->setTopPtr(newTopPtr);
}

void GC::IncCompactHeap(double timeBudget)
{
	//Reset the end of the heap to account for all allocations
	//that happened since the last frame
	compactor->SetIncHeapEnd(nurGenPtr);

	//Do incremental compaction
	void* newTopPtr = compactor->IncrementalCompact(allocator->basePtr, timeBudget);

	//Compaction completed
	if (newTopPtr)
	{
		collectorFlags = GC_NURSERY_COLLECT_NEXT | GC_TRACING;
		promoBuffEndPtr = newTopPtr;
		//double temp = lastColStamp; //Demo use only
		//lastColStamp = GCTimer::CurrentTimeInMS(); //Demo use only
		//lastColDur = lastColStamp - temp; //Demo use only
		//numCols++; //Demo use only
	}
}

void GC::TraceFullHeap()
{
	tracer->TraceOldGen(firstHandle);
}

void GC::FullOldGenCollection()
{
	MarkAllWhite();
	tracer->TraceOldGen(firstHandle);
	CompactHeap();
}

void GC::NurseryCollect()
{
	// Check we've allocated something
	if (LastAllocIndx == INDEX_SENTINEL)
		return;
	tracer->TraceNursery((Managed_Object*)nurGenPtr, (Managed_Object*)GC::Get(LastAllocIndx), &nurseryRoots);
	// Check if we have an older generation
	if (GC::LastPromotedIndx == INDEX_SENTINEL)
	{
		allocator->topPtr = compactor->NurseryCompact(allocator->lowerBoundPtr, allocator->topPtr, allocator->basePtr);
	}
	else
	{
		//We have an older generation
		//Compact against the promotion buffer
		allocator->topPtr = compactor->NurseryCompact(promoBuffEndPtr, allocator->topPtr, nurGenPtr);
	}
	nurGenPtr = promoBuffEndPtr = allocator->topPtr;
}

void GC::RegisterNurseryRoot( unsigned int index )
{
	for (unsigned int i = 0; i < static_cast<unsigned int>(nurseryRoots.size()); i++)
	{
		if (nurseryRoots[i] == INDEX_SENTINEL)
		{
			nurseryRoots[i] = index;
			return;
		}
	}
	nurseryRoots.push_back(index);
}

void GC::DeregisterNurseryRoot( unsigned int index )
{
	for (unsigned int i = 0; i < static_cast<unsigned int>(nurseryRoots.size()); i++)
	{
		if (nurseryRoots[i] == index)
		{
			nurseryRoots[i] = -1;
			return;
		}
	}
	throw std::exception("Index not found in nursery roots to unregister");
}

void GC::ClearNurseryRoots()
{
	nurseryRoots.clear();
}

unsigned int GC::Add(void *p)
{
	if (!p)
		return INDEX_SENTINEL;
	if (!refTable)
	{
		refTable = new std::vector<RefEntry>();
	}
	//Use the first empty slot in the table
	for (unsigned int i = 0; i < static_cast<unsigned int>(refTable->size()); i++)
	{
		if ((*refTable)[i].ptr == NULL)
		{
			(*refTable)[i].ptr = p;
			(*refTable)[i].Flags = ((*refTable)[i].Flags & (GC_ARRAY_FLAG | GC_HANDLE_ARRAY_FLAG)) | GC_GREEN_FLAG;
			return i;
		}
	}
	//If table is full, check we won't overflow
	if (refTable->size() == INDEX_SENTINEL)
		throw std::exception("Exceeded maximum number of managed objects");
	RefEntry nEntry;
	nEntry.ptr = p;
	nEntry.Flags = GC_GREEN_FLAG;
	refTable->push_back(nEntry);
	return (static_cast<unsigned int>(refTable->size()) - 1);
}

void* GC::Get(unsigned int i)
{
	if (!refTable)
		return NULL;
	if (i >= static_cast<unsigned int>(refTable->size()))
		return NULL;
	return (*refTable)[i].ptr;
}

void GC::RemoveRefAt(unsigned int index)
{
	if (!refTable)
		return;
	if (index >= static_cast<unsigned int>(refTable->size()))
		return;
	(*refTable)[index].ptr = NULL;
	(*refTable)[index].Flags = 0;
}

void GC::UpdateRef( unsigned int index, void* p)
{
	if (!refTable)
		return;
	if (index >= static_cast<unsigned int>(refTable->size()))
		return;
	(*refTable)[index].ptr = p;
}

bool GC::isColour( unsigned int index, unsigned int colour )
{
	return isFlagSet(index, colour);
}

bool GC::isArray( unsigned int index )
{
	return isFlagSet(index, GC_ARRAY_FLAG);
}

inline bool GC::isFlagSet( unsigned int index, unsigned int flag )
{
	if (refTable)
	{
		if (index >= static_cast<unsigned int>(refTable->size()))
			return false;
		RefEntry entry = (*refTable)[index];
		if (entry.ptr == NULL)
			return false;
		if (entry.Flags & flag)
			return true;
	}
	return false;
}

void GC::SetColour( unsigned int index, unsigned int colour )
{
	if (!refTable)
		return;
	if (index >= static_cast<unsigned int>(refTable->size()))
		return;
	if ((*refTable)[index].ptr == NULL)
		return;
	(*refTable)[index].Flags = ((*refTable)[index].Flags & (GC_ARRAY_FLAG | GC_HANDLE_ARRAY_FLAG)) | colour;
}

void GC::SetArrayFlag( unsigned int index )
{
	if (!refTable)
		return;
	if (index >= static_cast<unsigned int>(refTable->size()))
		return;
	if ((*refTable)[index].ptr == NULL)
		return;
	(*refTable)[index].Flags = (*refTable)[index].Flags | GC_ARRAY_FLAG;
}

void GC::SetHandleArrayFlag( unsigned int index )
{
	if (!refTable)
		return;
	if (index >= static_cast<unsigned int>(refTable->size()))
		return;
	if ((*refTable)[index].ptr == NULL)
		return;
	(*refTable)[index].Flags = (*refTable)[index].Flags | GC_HANDLE_ARRAY_FLAG;
}

void GC::MarkAllWhite()
{
	if (!refTable)
		return;
	for (unsigned int i = 0; i < static_cast<unsigned int>(refTable->size()); i++)
	{
		if ((*refTable)[i].ptr != NULL)
			(*refTable)[i].Flags = ((*refTable)[i].Flags & (GC_ARRAY_FLAG | GC_HANDLE_ARRAY_FLAG)) | GC_WHITE_FLAG;
	}
}

//Add object to pinned object linked list
Managed_Object* GC::AddToPinnedList( Managed_Object* pinnedObj )
{
	if (!pinnedObj)
		throw std::exception("Cannot add NULL pointer to pinned list");
	lastObjPinned = false;
	if (firstPinnedObj)
	{
		Managed_Object* pinnedObjPrev = (Managed_Object*)firstPinnedObj;
		Managed_Object* firstProxy = (Managed_Object*)Get(((Managed_Object*)firstPinnedObj)->getIndex());
		firstProxy->lastHandleOffset() = pinnedObj;
		firstPinnedObj = pinnedObj;
		if (!lowestPinnedAddr)
			throw std::exception("Error: lowestPinnedAddr should not be NULL at this point");
		if ((char*)pinnedObj < (char*)lowestPinnedAddr)
		{
			lowestPinnedAddr = pinnedObj;
		}
		if (!highestPinnedAddr)
			throw std::exception("Error: highestPinnedAddr should not be NULL at this point");
		if (((char*)pinnedObj->getUpperBound()) > highestPinnedAddr)
		{
			highestPinnedAddr = ((char*)pinnedObj->getUpperBound());
		}
		return pinnedObjPrev;
	}
	else
	{
		firstPinnedObj = pinnedObj;
		lowestPinnedAddr = (void*)pinnedObj;
		highestPinnedAddr = pinnedObj->getUpperBound();
		return NULL;
	}
}

void GC::RemoveFromPinnedList(Managed_Object* pinnedObj , Managed_Object* nextProxiedObj)
{
	if (!pinnedObj)
		return;
	if (pinnedObj->prevObj != NULL)
	{
		Managed_Object* prevProxy = (Managed_Object*)Get(pinnedObj->prevObj->getIndex());
		prevProxy->nextProxiedObj() = nextProxiedObj;
	}
	if (nextProxiedObj)
	{
		((Managed_Object*)nextProxiedObj)->prevObj = pinnedObj->prevObj;
	}
}

void* GC::GetTopHeapPtr()
{
	return allocator->topPtr;
}

void* GC::GetBaseHeapPtr()
{
	return allocator->basePtr;
}

Managed_Object* GC::GetLastAllocObj()
{
	if (GC::LastAllocIndx == INDEX_SENTINEL)
		return NULL;
	return (Managed_Object*)((char*)allocator->topPtr - lastAllocSize);
}

bool GC::IsOnHeap( void* addr )
{
	if (!addr)
		return false;
	if (!allocator)
		return false;
	return ((char*)addr >= (char*)allocator->basePtr 
		&& (char*)addr < (char*)allocator->topPtr);
}

bool GC::MightBePinned( void* addr )
{
	if (!(addr && lowestPinnedAddr && highestPinnedAddr))
		return false;
	return ((char*)addr >= (char*)lowestPinnedAddr 
		&& (char*)addr < (char*)highestPinnedAddr);
}







