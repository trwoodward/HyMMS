#pragma once

#include <windows.h>
#include "inputmanager.h"

class WindowsInputManager :
	public InputManager
{
public:
	WindowsInputManager(HWND);
	~WindowsInputManager(void);

	void GetMousePosition(float& x, float& y);
	bool IsLeftMouseButtonDown();

	bool IsKeyDown(int keyID);

private:
	HWND m_hwnd;
};

