#include "stdafx.h"
#include "GameObject.h"


GameObject::GameObject(void)
{
	position[0] = 0.0f;
	position[1] = 0.0f;
	origin[0] = 0.0f;
	origin[1] = 0.0f;
	scale[0] = 1.0f;
	scale[1] = 1.0f;
	rotation = 0.0f;
	texture = NULL;
	srcRect = DemoRectangle();
	tint = DemoColour();
	destroyer = false;
	index = -1;
}


GameObject::~GameObject(void)
{
}
