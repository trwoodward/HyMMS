#pragma once
#include "SmartHandle.h"
#include "Unmanaged_Array.h"

template<typename T>
class UnmanagedArrHandle :
	public SmartHandle<Unmanaged_Array<T> >
{
public:
	UnmanagedArrHandle<T>(void){}
	UnmanagedArrHandle<T>(Unmanaged_Array<T>* arr) : SmartHandle<Unmanaged_Array<T> >(arr){}
	~UnmanagedArrHandle<T>(void);

	Unmanaged_Array<T>& operator*();
	const Unmanaged_Array<T>& operator*() const;

	T& operator[](const int reqIndex);

private:
	// Made private to prevent misuse.
	UnmanagedArrHandle<T>(const SmartHandle<T>&);
	UnmanagedArrHandle<T>(Managed_Array<T>*);
};

template<typename T>
const Unmanaged_Array<T>& UnmanagedArrHandle<T>::operator*() const
{
	Unmanaged_Array<T>* arr = (Unmanaged_Array<T>*)GC::Get(index);
	if (!arr)
		throw std::exception("Attempted to de-reference NULL");
	if (!arr->isProxy)
		return *arr;
	else
		return *((Unmanaged_Array<T>*)arr->proxiedObj());
}

template<typename T>
Unmanaged_Array<T>& UnmanagedArrHandle<T>::operator*()
{
	Unmanaged_Array<T>* arr = (Unmanaged_Array<T>*)GC::Get(index);
	if (!arr)
		throw std::exception("Attempted to de-reference NULL");
	if (!arr->isProxy)
		return *arr;
	else
		return *((Unmanaged_Array<T>*)arr->proxiedObj());
}

template<typename T>
UnmanagedArrHandle<T>::~UnmanagedArrHandle(void)
{
}

template<typename T>
T& UnmanagedArrHandle<T>::operator[](const int reqIndex)
{
	if (index != INDEX_SENTINEL)
	{
		Unmanaged_Array<T>* base = (Unmanaged_Array<T>*)GC::Get(index);
		if (!base)
			throw std::exception("Cannot use subscript operator on uninitialized handle");
		if (base->isProxy)
			base = (Unmanaged_Array<T>*)base->proxiedObj();
		return *((T*)((char*)base + sizeof(Unmanaged_Array<T>) + base->firstObjPadding + (reqIndex * sizeof(T))));
	}
	throw std::exception("Handle not referencing an Unmanaged_Array");
}

