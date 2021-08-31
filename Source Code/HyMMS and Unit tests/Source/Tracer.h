#ifndef _TRACER_H_
#define  _TRACER_H_
#pragma once

#include "GCTimer.h"
#include <vector>

//State flags for incremental tracing
#define GC_ROOTS_TRACED 1
#define GC_GREY_FOUND 2
#define GC_REVERSE_PARSE 4

class Managed_Object;

class Tracer
{
public:
	Tracer(void);
	~Tracer(void);

private:
	//Main tracing functions
	void TraceOldGen(void* firstHandle);
	void TraceAllRoots(void* firstHandle);
	double IncrementalTrace(double msBudget);
	void TraceNurseryRoots(std::vector<unsigned int>* nurseryRoots);
	void TraceNursery(Managed_Object* firstObj, Managed_Object* lastObj, std::vector<unsigned int>* nurseryRoots);

	//Shared helper functions
	void MarkChildrenGrey(Managed_Object*);
	void TraceHeap(Managed_Object* firstObj, void* lastAddr, bool nursery);

	char TracerFlags;

	friend class GC;
	friend class TestSpy;
};

#endif