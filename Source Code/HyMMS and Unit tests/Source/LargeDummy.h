#pragma once
#include "dummy.h"
class LargeDummy :
	public Dummy
{
public:
	LargeDummy(void);
	~LargeDummy(void);

	//For large obj reasons.
	std::size_t MegaByteArray[131072];
};

