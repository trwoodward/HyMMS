#pragma once
#include <map>
#include <string>
#include <windows.h>

//Used for supervisor demo, not used for final project report
class DemoDebugInfo
{
public:
	DemoDebugInfo(HDC);
	~DemoDebugInfo(void);

	void DrawInfoToScreen();
	void AddDebugInfo(std::string name, double metric = 0.0);
	void SetDebugInfo(const std::string& key, double metric);
	void UpdateFrameRate(double elapsedTime);
private:
	std::map<std::string, double> debugInfo;
	HDC deviceContext;
	double debugDrawTime;

	double frameTimes[60];
	int frameCount;
};

