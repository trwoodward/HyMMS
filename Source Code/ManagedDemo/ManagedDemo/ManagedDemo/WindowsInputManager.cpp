#include "stdafx.h"
#include "WindowsInputManager.h"


WindowsInputManager::WindowsInputManager(HWND hwnd)
{
	m_hwnd = hwnd;
}


WindowsInputManager::~WindowsInputManager(void)
{
}

void WindowsInputManager::GetMousePosition( float& x, float& y )
{
	POINT p;
	if (GetCursorPos(&p))
	{
		if (ScreenToClient(m_hwnd, &p))
		{
			x = (float)p.x;
			y = (float)p.y;
			return;
		}
	}
	x = -1.0f;
	y = -1.0f;
}

bool WindowsInputManager::IsLeftMouseButtonDown()
{
	return ((GetKeyState(VK_LBUTTON) & 0x80) != 0);
}

bool WindowsInputManager::IsKeyDown( int keyID )
{
	return ((GetKeyState(keyID) & 0x80) != 0);
}
