#include "stdafx.h"
#include "DemoDebugInfo.h"
#include <sstream>
#include <iomanip>
#include <gl/GL.h>
#include "GCTimer.h"

//All code here used for demonstration to supervisor only

DemoDebugInfo::DemoDebugInfo(HDC hDC)
{
	deviceContext = hDC;
	debugInfo["FrameRate"] = 0.0;
	frameCount = 0;
	for (int i = 0; i < 10; ++i)
	{
		frameTimes[i] = 0.0;
	}
	debugDrawTime = 0.0;
}


DemoDebugInfo::~DemoDebugInfo(void)
{
}

void DemoDebugInfo::DrawInfoToScreen()
{
	double debugDrawDelta = GCTimer::CurrentTimeInMS();
	//Push all metrics into a single stream
	std::ostringstream debugStream;
	/*std::map<std::string, double>::iterator debugIterator = debugInfo.begin();
	for (; debugIterator != debugInfo.end(); ++debugIterator)
	{
		debugStream << debugIterator->first << ": " << debugIterator->second << std::endl;
	}*/
	std::string debugString;
	wglUseFontBitmaps(deviceContext, 0, 255, 1000);
	glListBase(1000);
	glPushMatrix();
	int textPos = 300;
	std::map<std::string, double>::iterator debugIterator = debugInfo.begin();
	for (; debugIterator != debugInfo.end(); ++debugIterator)
	{
		debugStream.str("");
		debugStream << debugIterator->first << ": " << std::fixed << std::setprecision(4) << debugIterator->second;
		debugString = debugStream.str();
		glRasterPos2d(0, textPos);
		glCallLists(debugString.size(), GL_UNSIGNED_BYTE, debugString.c_str());
		textPos += 15;
	}
	glPopMatrix();
	debugDrawTime = GCTimer::CurrentTimeInMS() - debugDrawDelta;
}

void DemoDebugInfo::AddDebugInfo( std::string name, double metric)
{
	debugInfo[name] = metric;
}

void DemoDebugInfo::SetDebugInfo( const std::string& key, double metric )
{
	debugInfo[key] = metric;
}

void DemoDebugInfo::UpdateFrameRate( double elapsedTime )
{
	frameTimes[frameCount] = elapsedTime - debugDrawTime;
	++frameCount;
	if (frameCount >= 60)
		frameCount = 0;

	double sum = 0.0;
	for (int i = 0; i < 60; ++i)
	{
		sum += frameTimes[i];
	}
	if (frameCount == 0)
		debugInfo["FrameRate"] = (sum / 60.0);
}
