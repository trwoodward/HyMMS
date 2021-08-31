#include "stdafx.h"
#include "ToggleButton.h"


ToggleButton::ToggleButton(DemoRectangle rect, Texture2D* tex, DemoColour col) : DemoButton(rect, tex, col)
{
	isPressed = false;
}


ToggleButton::~ToggleButton(void)
{
}

void ToggleButton::Depress()
{
	isPressed = !isPressed;
	if (isPressed)
		demo->selectDestroyer = true;
	else
	{
		demo->selectDestroyer = false;
	}
}

void ToggleButton::Unpress()
{
}
