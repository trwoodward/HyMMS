#pragma once
#ifndef _DEMOBUTTON_H_
#define _DEMOBUTTON_H_

#include "DemoRectangle.h"
#include "Texture2D.h"
#include "DemoColour.h"


class DemoButton
{
public:
	DemoButton(DemoRectangle rect, Texture2D* tex, DemoColour col);
	~DemoButton(void);

	bool IsInBounds(float x, float y);
	virtual void Depress() = 0;
	virtual void Unpress() = 0;

protected:
	friend class Demo;

	DemoRectangle bounds;
	bool isPressed;
	Texture2D* releasedTex;
	Texture2D* pressedTex;
	DemoColour col;
	Demo* demo;
};

#endif
