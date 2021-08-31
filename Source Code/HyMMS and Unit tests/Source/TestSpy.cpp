#include "stdafx.h"
#include "TestSpy.h"
#include "GC.h"
#include "Tracer.h"


TestSpy::TestSpy(void)
{
}


TestSpy::~TestSpy(void)
{
}

void TestSpy::SetAllHandleResults( SmartHandle<Dummy> obj, int r )
{
	Managed_Object* actualObj = &(*obj);
	SmartHandle<Dummy>* currHandle;
	if (actualObj->firstHandleOffset() != NULL)
		currHandle = (SmartHandle<Dummy>*)((char*)actualObj + (unsigned int)(actualObj->firstHandleOffset()));
	else
		currHandle = NULL;
	while (currHandle != NULL)
	{
		if (currHandle->index != INDEX_SENTINEL)
			(*currHandle)->setResult(r);
		if ((*currHandle).nextHndl > 0)
			currHandle = (SmartHandle<Dummy>*)((char*)actualObj + (unsigned int)(*currHandle).nextHndl);
		else
			currHandle = NULL;
	}
}

void TestSpy::SetAllRootResults( int r )
{
	if (GC::firstHandle == NULL)
		return;
	SmartHandle<Dummy>* currHandle = (SmartHandle<Dummy>*)GC::firstHandle;
	while (currHandle != NULL)
	{
		if (currHandle->index != INDEX_SENTINEL)
		{
			(*currHandle)->setResult(r);
		}
		currHandle = (SmartHandle<Dummy>*)currHandle->nextHndl;
	}
}

void* TestSpy::GetPointer( int index )
{
	return GC::Get(index);
}

void* TestSpy::ForceAllocate( std::size_t reqSize )
{
	return GC::allocator->allocate(reqSize, false);
}

void* TestSpy::GetHeapTopPtr()
{
	return GC::allocator->topPtr;
}

void* TestSpy::ForceAllocateAligned( std::size_t reqSize, char alignment )
{
	return GC::AllocateAligned(reqSize, alignment, false);
}

bool TestSpy::IsOnHeap( void* addr )
{
	return GC::IsOnHeap(addr);
}

void TestSpy::MarkChildrenGrey( Managed_Object* parent )
{
	GC::tracer->MarkChildrenGrey(parent);
}

bool TestSpy::isGrey( Managed_Object* obj )
{
	return GC::isColour(obj->index, GC_GREY_FLAG);
}

bool TestSpy::isBlack( Managed_Object* obj )
{
	return GC::isColour(obj->index, GC_BLACK_FLAG);
}

bool TestSpy::isGreen( Managed_Object* obj )
{
	return GC::isColour(obj->index, GC_GREEN_FLAG);
}

bool TestSpy::isWhite( Managed_Object* obj )
{
	return GC::isColour(obj->index, GC_WHITE_FLAG);
}

void TestSpy::TraceAllRoots()
{
	GC::tracer->TraceAllRoots(GC::firstHandle);
}

void TestSpy::TraceNurseryRoots()
{
	GC::tracer->TraceNurseryRoots(&GC::nurseryRoots);
}

void TestSpy::TraceNursery()
{
	GC::tracer->TraceNursery((Managed_Object*)GC::nurGenPtr, GC::GetLastAllocObj(), &GC::nurseryRoots);
}

void TestSpy::TraceOldGen()
{
	GC::tracer->TraceOldGen(GC::firstHandle);
}

void* TestSpy::NurseryCompact()
{
	return GC::compactor->NurseryCompact(GC::GetBaseHeapPtr(), GC::GetTopHeapPtr(), GC::allocator->basePtr);
}

void TestSpy::NurseryCollection()
{
	GC::NurseryCollect();
}

void TestSpy::FullCollection()
{
	GC::CollectAll();
}



