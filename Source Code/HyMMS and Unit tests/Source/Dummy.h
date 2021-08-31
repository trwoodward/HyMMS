#pragma once
#include "SmartHandle.h"
#include "Managed_Object.h"

// Dummy class used for testing features of SmartHandles
class Dummy : public Managed_Object
{
public:
	Dummy(void);
	~Dummy(void);

	//Used to test general method calls
	void generalMethod();

	//Used to test const arrow and de-reference operators

	void publicConstMethod() const;

	//Used to verify test results
	int getResult();
	void setResult(int);

	//Used to test const arrow and de-reference operators
	void SetDummy(SmartHandle<Dummy> h);

	SmartHandle<Dummy> h1;
	SmartHandle<Dummy> h2;
	SmartHandle<Dummy> h3;

private:
	//Used to determine which methods are called
	int result;

	//Used to test const arrow and de-reference operators
	SmartHandle<Dummy> constDummy;
	void privateConstMethod() const;
};

