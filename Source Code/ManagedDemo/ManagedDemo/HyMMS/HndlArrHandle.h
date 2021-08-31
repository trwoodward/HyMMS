#pragma once
#include "SmartHandle.h"
#include "Handle_Array.h"

//Work around for lack of template aliases in MSVC
template<typename T>
class HndlArrHandle :
	public UnmanagedArrHandle<SmartHandle<T> >
{
public:
	HndlArrHandle<T>(void) : UnmanagedArrHandle<SmartHandle<T> >() {}
	HndlArrHandle<T>(Handle_Array<T>* arr) : UnmanagedArrHandle<SmartHandle<T> >(arr) {}
};


