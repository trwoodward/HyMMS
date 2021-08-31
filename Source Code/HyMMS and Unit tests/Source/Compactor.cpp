#include "stdafx.h"
#include "Compactor.h"
#include "Managed_Object.h"
#include "GC.h"
#include "GCUtil.h"
#include "Managed_Array.h"
#include "Handle_Array.h"
#include <chrono>


Compactor::Compactor(void)
{
	incHeapCursor = NULL;
	incHeapCumulative = 0;
	currArr = NULL;
}


Compactor::~Compactor(void)
{
}

//Compacts the heap and returns a pointer to the new top of the heap
void* Compactor::Compact(void* heapStart, void* heapEnd, void*& firstObject)
{
	if (!heapStart || !heapEnd)
		throw std::exception("Invalid heap boundary");

	if (!firstObject) //Assume no objects on the heap
		return heapStart;

	void* cursor = firstObject;
	void* newTopPtr = heapStart;
	std::size_t cumulative = 0;

	Managed_Object* prevLiveObj = NULL;

	while ((char*)cursor < (char*)heapEnd)
	{
		Managed_Object* current = (Managed_Object*)cursor;
		unsigned int currIndex = current->getIndex();
		if (!GC::isColour(currIndex, GC_WHITE_FLAG))
		{
			//Object is alive

			//Update it's prevObj pointer
			current->prevObj = prevLiveObj;

			//Calculate new address of object
			void* newAddr = HandleLiveObject(cursor, prevLiveObj, cumulative, firstObject, newTopPtr);

			//Set object to white
			GC::SetColour(currIndex, GC_WHITE_FLAG);
		}
		else 
		{
			HandleDeadObject(cursor, cumulative, prevLiveObj, firstObject, heapStart);
			//Call object destructor
			delete current;
		}
	}
	if(prevLiveObj != NULL)
	{
		//Set LastAllocIndx to the index of new last object on the heap.
		GC::LastAllocIndx = prevLiveObj->index;
		GC::LastPromotedIndx = GC::LastAllocIndx;
		//Set last object on the heap's padding to zero
		prevLiveObj->nextObjPadding = 0;
	}
	return newTopPtr;
}


void Compactor::SetIncHeapStart(void* start)
{
	if (!start)
		throw std::exception("HeapStart cannot be NULL");
	incHeapStart = start;
}

void Compactor::SetIncHeapEnd(void* end)
{
	if (!end)
		throw std::exception("HeapEnd cannot be NULL");
	incHeapEnd = end;
}

void* Compactor::IncrementalCompact(void* &incFirstObjPtr, double msBudget)
{
	if(!(incHeapStart && incHeapEnd && incFirstObjPtr))
	{
		//both incHeapStart and incHeapEnd need to be set
		//before incremental compaction can take place.
		return NULL;
	}

	if (!incHeapCursor)
	{
		if(incFirstObjPtr >= incHeapEnd)
			return NULL;

		//We're starting a new compaction
		incHeapCursor = incFirstObjPtr;
		incHeapCumulative = 0;
		incNewTopPtr = incHeapStart;
		incPrevLiveObj = NULL;
	}

	double startTime = GCTimer::CurrentTimeInMS();
	double currTime = startTime;
	bool done = false;

	while (currTime - startTime < msBudget)
	{
		Managed_Object* current = (Managed_Object*)incHeapCursor;
		unsigned int currIndex = current->getIndex();
		if (!GC::isColour(currIndex, GC_WHITE_FLAG))
		{
			//Handle this being an array
			if (currArr)
			{
				++(currArr->breakAt);
				if (currArr->breakAt == currArr->length)
				{
					currArr->breakAt = -1;
					Managed_Array<Managed_Object>* prevArr = currArr;
					currArr = (Managed_Array<Managed_Object>*)currArr->nextArray();
					prevArr->nextArray() = NULL;
				}
			}

			if (GC::isArray(current->getIndex()) && incHeapCumulative > 0)
			{
				Managed_Array<Managed_Object>* prevArr = currArr;
				currArr = (Managed_Array<Managed_Object>*)current;
				currArr->nextArray() = prevArr;
				currArr->breakAt = 0;
			}

			//Object is alive
			void* newAddr = HandleLiveObject(incHeapCursor, incPrevLiveObj, incHeapCumulative, incFirstObjPtr, incNewTopPtr);

			//Set object to white
			GC::SetColour(currIndex, GC_WHITE_FLAG);

			//Check if this was the last obj on the heap or the old gen
			if ((char*)incHeapCursor >= (char*)incHeapEnd)
			{
				GC::LastPromotedIndx = currIndex;
				((Managed_Object*)newAddr)->nextObjPadding = 0;
				done = true;
				break;
			}
			else
			{
				((Managed_Object*)incHeapCursor)->prevObj = (Managed_Object*)newAddr;
			}
		}
		else 
		{
			HandleDeadObject(incHeapCursor, incHeapCumulative, incPrevLiveObj, incFirstObjPtr, incHeapStart);
			//Check if this was the last obj on the heap or the old gen
			if ((char*)incHeapCursor >= (char*)incHeapEnd)
			{
				//Check if we've deleted the last object
				if (currIndex == GC::LastAllocIndx)
				{
					if (incPrevLiveObj != NULL)
					{
						GC::LastAllocIndx = incPrevLiveObj->getIndex();
					}
					else
					{
						GC::LastAllocIndx = INDEX_SENTINEL;
					}
					GC::LastPromotedIndx = GC::LastAllocIndx;
				}
				else if (currIndex == GC::LastPromotedIndx) //Or last object in old gen
				{
					if (incPrevLiveObj != NULL)
					{
						GC::LastPromotedIndx = incPrevLiveObj->getIndex();
					}
					else
					{
						GC::LastPromotedIndx = INDEX_SENTINEL;
					}
				}
				done = true;
				break;
			}
			else
			{
				((Managed_Object*)incHeapCursor)->prevObj = current->prevObj;
			}	
			//Call object destructor
			delete current;
		}
		//Refresh timestamp
		currTime = GCTimer::CurrentTimeInMS();
	}
	if (done)
	{
		incHeapCursor = NULL;
		return incNewTopPtr;
	}
	else
	{
		return NULL;
	}
}

void* Compactor::NurseryCompact( void* oldGenEnd, void* heapEnd, void*& firstObject)
{
	if (!oldGenEnd || !heapEnd)
		throw std::exception("Invalid heap boundary");

	if (!firstObject) //Assume no objects in the Nursery
		return oldGenEnd;

	void* cursor = firstObject;
	void* newTopPtr = oldGenEnd;

	std::size_t cumulative = (char*)cursor - (char*)newTopPtr;

	Managed_Object* prevLiveObj = NULL;
	if (GC::LastPromotedIndx != INDEX_SENTINEL)
	{
		prevLiveObj = (Managed_Object*)GC::Get(GC::LastPromotedIndx);
	}

	while ((char*)cursor < (char*)heapEnd)
	{
		Managed_Object* current = (Managed_Object*)cursor;
		unsigned int currIndex = current->getIndex();
		if (!GC::isColour(currIndex, GC_GREEN_FLAG))
		{
			//Object is alive
			current->prevObj = prevLiveObj;

			//Calculate new address of object
			void* newAddr = HandleLiveObject(cursor, prevLiveObj, cumulative, firstObject, newTopPtr);

			//Remove IN_NURSERY flag from all SmartHandles
			((Managed_Object*)newAddr)->removeNurseryFlags();
		}
		else 
		{
			HandleDeadObject(cursor, cumulative, (void*)1, firstObject, NULL);

			//Call object destructor
			current->~Managed_Object();
		}
	}
	if(prevLiveObj != NULL)
	{
		//Set LastAllocIndx to the index of new last object on the heap.
		GC::LastAllocIndx = prevLiveObj->index;
		//Set last object on the heap's padding to zero
		prevLiveObj->nextObjPadding = 0;
	}
	GC::ClearNurseryRoots();
	return newTopPtr;
}

void* Compactor::HandleLiveObject(void* &cursor, Managed_Object* &prevLiveObj, std::size_t& cumulative, void* &firstObject, void* &newTopPtr )
{
	if (!cursor)
		throw std::exception("Cursor cannot be NULL");
	if (!firstObject)
		throw std::exception("We cannot have a live object if there is no firstObject");

	Managed_Object* current = (Managed_Object*)cursor;
	std::size_t currObjSize = current->getSize();
	unsigned int currIndex = current->getIndex();
	char currNxtObjPadding = current->getNextObjPadding();

	//Calculate new address of object
	void* newAddr = (void*)(((char*)cursor) - cumulative);
	cumulative += currNxtObjPadding;

	char adjustment = CalculateAdjustment(newAddr, current->objAlignment);
	newAddr = (char*)newAddr + adjustment;
	cumulative -= adjustment;
	if (prevLiveObj != NULL)
	{
		//Update padding element of prevObject
		prevLiveObj->nextObjPadding = adjustment;
	}

	//If just overwriting the firstObject, update allocator's ref
	if (prevLiveObj == NULL && cursor != firstObject)
	{
		firstObject = newAddr;
	}

	//Calculate new topPtr
	newTopPtr = (void*)(((char*)newAddr) + currObjSize);

	//cache address of next object
	cursor = (void*)(((char*)cursor) + currObjSize + currNxtObjPadding);

	//Move current object to new address
	if (newAddr != (void*)(&current))
	{
		//update reference table
		GC::UpdateRef(currIndex, newAddr);
		memmove(newAddr, current, currObjSize);
	}

	//Set prevLiveObj
	prevLiveObj = (Managed_Object*)newAddr;

	return newAddr;
}

void Compactor::HandleDeadObject( void* &cursor, std::size_t& cumulative, void* prevLiveObj, void* firstObject, void* heapStart )
{
	if (!cursor)
		throw std::exception("Cursor cannot be NULL");
	if (!firstObject)
		throw std::exception("We cannot have a live object if there is no firstObject");

	Managed_Object* current = (Managed_Object*)cursor;
	unsigned int currIndex = current->getIndex();
	std::size_t currObjSize = current->getSize();
	char currNxtObjPadding = current->getNextObjPadding();

	//Object is dead
	GC::RemoveRefAt(currIndex);

	//Add size to cumulative
	cumulative += currObjSize + currNxtObjPadding;

	//If first object is dead, add the preceding allocation padding
	if(prevLiveObj == NULL && cursor != firstObject)
	{
		cumulative += (char*)firstObject - (char*)heapStart;
	}

	//Increment cursor
	cursor = (void*)(((char*)cursor) + currObjSize + currNxtObjPadding);
}




