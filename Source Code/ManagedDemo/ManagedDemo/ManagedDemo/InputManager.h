#pragma once
class InputManager
{
public:
	InputManager(void);
	~InputManager(void);

	virtual void GetMousePosition(float& x, float& y) = 0;
	virtual bool IsLeftMouseButtonDown() = 0;
};

