#include "stdafx.h"
#include "DemoRand.h"
#include <cstdlib>



double DemoRand::GetRandNormalized()
{
	double retVal;
	srand((unsigned int)GCTimer::CurrentTimeInMS());
	for (int i = 0; i < 10; ++i)
	{
		retVal = ((double)rand() / (RAND_MAX + 1));
	}
	return retVal;
}
