#pragma once
#include "DemoButton.h"

class PushButton :
	public DemoButton
{
public:
	PushButton(DemoRectangle rect, Texture2D* tex, DemoColour col);
	~PushButton(void);

	void Depress();
	void Unpress();
};

