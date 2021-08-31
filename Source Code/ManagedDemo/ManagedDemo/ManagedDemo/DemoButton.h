#pragma once
#ifndef _DEMOBUTTON_H_
#define _DEMOBUTTON_H_

#include "DemoRectangle.h"
#include "Texture2D.h"
#include "DemoColour.h"
#include "HyMMS.h"


class DemoButton : public Managed_Object
{
public:
	DemoButton(){}
	DemoButton(DemoRectangle rect, SmartHandle<Texture2D> tex, DemoColour col);
	~DemoButton(void);

	bool IsInBounds(float x, float y);
	virtual void Depress(){}
	virtual void Unpress(){}

protected:
	friend class Demo;

	DemoRectangle bounds;
	bool isPressed;
	SmartHandle<Texture2D> releasedTex;
	SmartHandle<Texture2D> pressedTex;
	DemoColour col;
	Demo* demo;
};

#endif
