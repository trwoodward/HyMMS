#pragma once
#include "Dummy.h"
#include "SmartHandle.h"

class TestSpy
{
public:
	TestSpy();

	void SetAllHandleResults(SmartHandle<Dummy> obj, int r);
	void SetAllRootResults(int r);

	void* GetPointer(int index);
	
	template<typename T>
	int GetIndex(SmartHandle<T> hndl);

	//Expose various internal functions of the system for testing purposes
	void* ForceAllocate(std::size_t);
	void* ForceAllocateAligned(std::size_t reqSize, char alignment);
	void* GetHeapTopPtr();
	bool IsOnHeap(void* addr);
	void MarkChildrenGrey(Managed_Object* parent);
	bool isGrey(Managed_Object* obj);
	bool isGreen(Managed_Object* obj);
	bool isBlack(Managed_Object* obj);
	bool isWhite(Managed_Object* obj);
	void TraceAllRoots();
	void TraceNurseryRoots();
	void TraceNursery();
	void TraceOldGen();
	void* NurseryCompact();
	void NurseryCollection();
	void FullCollection();


	~TestSpy();
};

template<typename T>
int TestSpy::GetIndex( SmartHandle<T> hndl )
{
	return hndl->index;
}


