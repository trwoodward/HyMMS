#pragma once
#include "SmartHandle.h"
#include "Dummy.h"

class UnmanagedHndlDummy
{
public:
	UnmanagedHndlDummy(void);
	~UnmanagedHndlDummy(void);

	SmartHandle<Dummy> hndl;
};

