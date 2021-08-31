#ifndef _HANDLE_ARRAY_H_
#pragma once

#include "Managed_Object.h"

//Work around for lack of template aliases in MSVC
template<typename T>
class Handle_Array :
	public Unmanaged_Array<SmartHandle<T> >
{
public:
	Handle_Array<T>(int count) : Unmanaged_Array<SmartHandle<T> >(count) {}
};

#endif

