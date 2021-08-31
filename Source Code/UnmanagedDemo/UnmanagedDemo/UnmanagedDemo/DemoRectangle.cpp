#include "stdafx.h"
#include "DemoRectangle.h"


DemoRectangle::DemoRectangle(void)
{
	x = y = width = height = 0;
}

DemoRectangle::DemoRectangle( int a_x, int a_y, int a_width, int a_height )
{
	x = a_x;
	y = a_y;
	width = a_width;
	height = a_height;
}


DemoRectangle::~DemoRectangle(void)
{
}
