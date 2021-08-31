#include "stdafx.h"
#include "Tracer.h"
#include "SmartHandle.h"
#include "GCUtil.h"
#include "Managed_Array.h"
#include "Handle_Array.h"



Tracer::Tracer(void)
{
	TracerFlags = 0;
}


Tracer::~Tracer(void)
{
}

void Tracer::TraceOldGen(void* firstHandle)
{
	TraceAllRoots(firstHandle);
	if (GC::LastPromotedIndx == INDEX_SENTINEL) // No objects in old gen
		return;
	TracerFlags = 0;
	TraceHeap((Managed_Object*)GC::GetBaseHeapPtr(), GC::promoBuffEndPtr, false);
}

void Tracer::TraceAllRoots( void* firstHandle )
{
	//Loop over roots list and mark each index they point to as Grey.
	if (firstHandle == NULL)
		return;
	SmartHandle<Managed_Object>* currHandle = (SmartHandle<Managed_Object>*)firstHandle;
	while (currHandle != NULL)
	{
		if (currHandle->index != INDEX_SENTINEL)
		{
			if (!GC::isColour(currHandle->index, GC_GREEN_FLAG))
			{
				GC::SetColour(currHandle->index, GC_GREY_FLAG);
			}
		}
		currHandle = (SmartHandle<Managed_Object>*)currHandle->nextHndl;
	}
}

double Tracer::IncrementalTrace(double msBudget)
{
	double prevTime = GCTimer::CurrentTimeInMS();
	double currTime = prevTime;
	double elapsedTime = 0.0;

	// We've only had a Nursery collection, need a full compaction before
	// we have an old gen to trace.
	if (GC::LastPromotedIndx == INDEX_SENTINEL)
		return msBudget;

	if (!(TracerFlags & GC_ROOTS_TRACED))
	{
		TraceAllRoots(GC::firstHandle);
		TracerFlags = TracerFlags | GC_ROOTS_TRACED;

		currTime = GCTimer::CurrentTimeInMS();
		elapsedTime = (currTime - prevTime);
		prevTime = currTime;
		if (elapsedTime > msBudget)
			return -1.0;
	}

	Managed_Object* currentObj;

	if (!(TracerFlags & GC_REVERSE_PARSE))
		currentObj = (Managed_Object*)GC::GetBaseHeapPtr();
	else
	{
		currentObj = (Managed_Object*)GC::Get(GC::LastPromotedIndx);
		if (!currentObj)
			throw std::exception("GC::LastPromotedIndx invalid (but not INDEX_SENTINEL)");
	}

	while (elapsedTime <= msBudget) //<= ensures loop will always run once even on zero budget
	{
		if (GC::isColour(currentObj->index, GC_GREY_FLAG))
		{
			TracerFlags = TracerFlags | GC_GREY_FOUND;
			MarkChildrenGrey(currentObj);
			GC::SetColour(currentObj->index, GC_BLACK_FLAG);
		}

		Managed_Object* nextObj = NULL;
		bool changeDir = false;
		if (!(TracerFlags & GC_REVERSE_PARSE))
		{
			nextObj = (Managed_Object*)((char*)currentObj + currentObj->objSize + currentObj->nextObjPadding);
			changeDir = (currentObj == ((Managed_Object*)GC::Get(GC::LastPromotedIndx)));
		}
		else
		{
			nextObj = currentObj->prevObj;
			changeDir = (currentObj == (Managed_Object*)GC::GetBaseHeapPtr());
		}

		if (changeDir)
		{
			if (TracerFlags & GC_GREY_FOUND)
			{
				// Change direction
				TracerFlags = TracerFlags ^ GC_REVERSE_PARSE;
				TracerFlags = TracerFlags & (~GC_GREY_FOUND);
			}
			else
			{
				TracerFlags = 0;
				currTime = GCTimer::CurrentTimeInMS();
				elapsedTime += (currTime - prevTime);
				double retVal = msBudget - elapsedTime;
				return (retVal > 0.0) ? retVal : 0.0;
			}
		}
		else
		{
			if (!nextObj)
				throw std::exception("Old gen tracer found NULL linkage before beginning/ end of heap");
			currentObj = nextObj;
		}

		currTime = GCTimer::CurrentTimeInMS();
		elapsedTime += (currTime - prevTime);
		prevTime = currTime;
	}
	
	return -1.0;
}

void Tracer::MarkChildrenGrey( Managed_Object* parent)
{
	if (GC::isArray(parent->index))
	{
		// Deref handle, up cast to Managed_Array
		// Traverse indices vector and mark all Grey
		Managed_Array<Managed_Object>* arr = (Managed_Array<Managed_Object>*)(GC::Get(parent->index));
		for (int i = 0; i < static_cast<int>((arr->indices).size()); i++)
		{
			// Mark all indices
			if (arr->indices[i] != INDEX_SENTINEL)
			{
				GC::SetColour(arr->indices[i], GC_GREY_FLAG);
			}
		}
	}

	if (parent->firstHandleOffset() != NULL)
	{
		if (parent->isProxy)
		{
			parent = (Managed_Object*)parent->proxiedObj();
			if (parent->firstHandleOffset() == NULL)
				return;
		}
		SmartHandle<Managed_Object>* hndl = (SmartHandle<Managed_Object>*)((char*)parent + (unsigned int)parent->firstHandleOffset());
		while (true)
		{
			if (hndl->index != INDEX_SENTINEL)
			{
				if (!GC::isColour(hndl->index, GC_BLACK_FLAG))
					GC::SetColour(hndl->index, GC_GREY_FLAG);
			}
			if (hndl->nextHndl > 0)
				hndl = (SmartHandle<Managed_Object>*)((char*)parent + (unsigned int)hndl->nextHndl);
			else
				break;
		}
	}
}

void Tracer::TraceNursery( Managed_Object* firstObj, Managed_Object* lastObj, std::vector<unsigned int>* nurseryRoots)
{
	if ((char*)firstObj > (char*)lastObj) // No objects in nursery
		return;

	TraceNurseryRoots(nurseryRoots);
	TraceHeap(firstObj, (char*)lastObj + lastObj->getSize(), true);
}

void Tracer::TraceNurseryRoots(std::vector<unsigned int>* nurseryRoots)
{
	if (!nurseryRoots)
		throw std::exception("Nursery roots argument cannot be NULL");
	for (unsigned int i = 0; i < static_cast<unsigned int>((*nurseryRoots).size()); i++)
	{
		if ((*nurseryRoots)[i] != INDEX_SENTINEL)
		{
			GC::SetColour((*nurseryRoots)[i], GC_GREY_FLAG);
		}
	}
}

void Tracer::TraceHeap( Managed_Object* firstObj, void* lastAddr, bool nursery )
{
	if (!firstObj || ! lastAddr || ((char*)firstObj >= (char*)lastAddr))
		throw std::exception("Invalid firstObj/ lastAddr arguments");
	bool forwards = true;
	bool greyFound = true;
	unsigned int DEAD_FLAG = nursery ? GC_GREEN_FLAG : GC_WHITE_FLAG;

	//Start with first object on the heap
	Managed_Object* currObj = firstObj;

	while (greyFound)
	{
		greyFound = !greyFound;

		if (GC::isColour(currObj->index, GC_GREY_FLAG))
		{
			greyFound = true;
			//Mark handles of the object
			MarkChildrenGrey(currObj);
			//Mark this object as Black
			GC::SetColour(currObj->index, GC_BLACK_FLAG);
		}
		else if (!(GC::isColour(currObj->index, GC_BLACK_FLAG) || GC::isColour(currObj->index, DEAD_FLAG)))
		{
			//Object with no marking found, throw error
			throw std::exception("Object with no colour marking found");
		}

		//Move on to the next object in the heap
		Managed_Object* nextObj = NULL;
		bool changeDir = false;
		if (forwards)
		{
			nextObj = (Managed_Object*)((char*)currObj + currObj->objSize);
			changeDir = ((char*)nextObj >= (char*)lastAddr);
		}
		else
		{
			nextObj = currObj->prevObj;
			changeDir = (nextObj == NULL);
		}
		if (!changeDir)
		{
			if (!nextObj)
				throw std::exception("Tracer has found NULL linkage not at an end of the heap");
			currObj = nextObj;
		}
		else
		{
			//Change direction
			forwards = !forwards;
		}
	}
}
