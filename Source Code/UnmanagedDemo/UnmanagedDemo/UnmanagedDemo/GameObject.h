#pragma once
#include "DemoRectangle.h"
#include "DemoColour.h"
#include "Texture2D.h"
#include <Box2D/Box2D.h>

class GameObject
{
public:
	float& Width() { return scale[0]; }
	float& Height() { return scale[1]; }

	GameObject(void);
	~GameObject(void);
	float position[2];
	float scale[2];
	float rotation;
	float origin[2];
	Texture2D* texture;
	DemoRectangle srcRect;
	DemoColour tint;
	b2Body* physBody;
	int index;

	bool destroyer;
};

