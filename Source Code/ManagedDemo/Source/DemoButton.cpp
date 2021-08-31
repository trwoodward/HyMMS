#include "stdafx.h"
#include "DemoButton.h"


DemoButton::DemoButton(DemoRectangle rect, SmartHandle<Texture2D> tex, DemoColour col) 
	: bounds(rect), col(col)
{
}


DemoButton::~DemoButton(void)
{
}

bool DemoButton::IsInBounds( float x, float y )
{
	if (x < bounds.X() || x > (bounds.X() + bounds.Width()) || y < bounds.Y() || y > (bounds.Y() + bounds.Height()))
		return false;
	return true;
}
