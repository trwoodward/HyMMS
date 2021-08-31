#pragma once

#include "GCTimer.h"
#include <vector>

//Used for performance metric gathering
class PerfInfo
{
public:
	PerfInfo(void);
	~PerfInfo(void);

	double timeSoFar;
	std::vector<double> times;
	int frameCount;

	void Tick(double ms);

	void StopAndPrint();
};

