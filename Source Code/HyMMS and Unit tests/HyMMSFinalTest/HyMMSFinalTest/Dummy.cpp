#include "stdafx.h"
#include "Dummy.h"

//Dummy class implementation

Dummy::Dummy(void)
{
	result = 0;
}


Dummy::~Dummy(void)
{
}

void Dummy::generalMethod()
{
	result = 1;
}

int Dummy::getResult()
{
	return result;
}

void Dummy::publicConstMethod() const
{
	//Used to test const version of arrow operator
	constDummy->privateConstMethod();
	//Used to test const version of de-reference operator
	(*constDummy).privateConstMethod();
}

void Dummy::privateConstMethod() const
{
}

void Dummy::SetDummy( SmartHandle<Dummy> h )
{
	constDummy = h;
}

void Dummy::setResult( int r)
{
	result = r;
}
