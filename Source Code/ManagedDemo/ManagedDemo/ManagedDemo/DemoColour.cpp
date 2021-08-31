#include "stdafx.h"
#include "DemoColour.h"


DemoColour::DemoColour(void)
{
	for (int i = 0; i < 4; ++i)
	{
		rgba[i] = 0.0f;
	}
}

DemoColour::DemoColour( float r, float g, float b, float a )
{
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
}


DemoColour::~DemoColour(void)
{
}
