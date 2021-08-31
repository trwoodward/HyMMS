#include "stdafx.h"
#include "PushButton.h"
#include "Demo.h"

PushButton::PushButton(DemoRectangle rect, Texture2D* tex, DemoColour col) : DemoButton(rect, tex, col)
{
	isPressed = false;
}


PushButton::~PushButton(void)
{
}

void PushButton::Depress()
{
	isPressed = true;
	demo->SpawnObject();
}

void PushButton::Unpress()
{
	isPressed = false;
}
