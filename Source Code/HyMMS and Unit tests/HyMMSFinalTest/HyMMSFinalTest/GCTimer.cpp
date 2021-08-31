#include "stdafx.h"
#include "GCTimer.h"
#ifdef  WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif //  WIN32

double GCTimer::ticksPerSec = -1.0;

double GCTimer::CurrentTimeInMS()
{
#ifdef WIN32
	//Lazily initialize ticksPerSec
	if (ticksPerSec < 0.0)
	{
		LARGE_INTEGER liTicksPerSec;

		//Get ticks per sec
		QueryPerformanceFrequency(&liTicksPerSec);

		//cache frequency
		ticksPerSec = liTicksPerSec.QuadPart;
	}

	LARGE_INTEGER currTime;

	//Get current timestamp
	QueryPerformanceCounter(&currTime);

	//return time in milliseconds
	return ((currTime.QuadPart * 1000.0) / ticksPerSec);
#else
	timeval currTime;

	//Get current timestamp
	gettimeofday(&currTime, NULL);

	//Calculate timestamp in milliseconds
	double retTime = currTime.tv_sec * 1000.0;
	retTime += currTime.tv_usec / 1000.0;

	return retTime;
#endif
}
