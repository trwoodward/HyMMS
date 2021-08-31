#pragma once
#include "demobutton.h"
#include "Demo.h"

class ToggleButton :
	public DemoButton
{
public:
	ToggleButton(DemoRectangle rect, Texture2D* tex, DemoColour col);
	~ToggleButton(void);

	void Depress();
	void Unpress();
};

