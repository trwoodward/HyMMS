#pragma once
#include <windows.h>

class GCTimer
{
public:
	static double CurrentTimeInMS();
private:
	static double ticksPerSec;

	friend double G_GetCurrTime();
};

//Helper function used to work around obscure bug 
//that occurs when too many static calls are made in 
//a very short space of time
inline double G_GetCurrTime()
{
	if (GCTimer::ticksPerSec < 0.0)
	{
		LARGE_INTEGER liTicksPerSec;

		//Get ticks per sec
		QueryPerformanceFrequency(&liTicksPerSec);

		//cache frequency
		GCTimer::ticksPerSec = liTicksPerSec.QuadPart;
	}
	LARGE_INTEGER testInt;
	QueryPerformanceCounter(&testInt);

	return ((testInt.QuadPart * 1000.0) / GCTimer::ticksPerSec);
}







