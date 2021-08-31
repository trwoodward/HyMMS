#include "stdafx.h"
#include "PerfInfo.h"
#include <fstream>


PerfInfo::PerfInfo(void)
{
	timeSoFar = 0.0;
	times = std::vector<double>(100000);
	for (int i = 0; i < 100000; ++i)
	{
		times[i] = 0.0;
	}
	times[0] = timeSoFar;
	frameCount = 1;
}


PerfInfo::~PerfInfo(void)
{
}

void PerfInfo::Tick( double ms )
{
	timeSoFar += ms;
	if (frameCount < 99999)
		times[frameCount++] = timeSoFar;
	else
		times.push_back(timeSoFar);
}

void PerfInfo::StopAndPrint()
{
	std::ofstream outputFile;
	outputFile.open("./NurseryCollectionTimes.csv");
	for (int i = 0; i < times.size(); ++i)
	{
		outputFile << i << "," << times[i] << std::endl;
	}
	outputFile.close();
}
