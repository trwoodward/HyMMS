#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include "stdafx.h"
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#include "Game.h"
#include "Demo.h"

//All code in this file is taken from or heavily based on code from
//Beginning OpenGL Programming by Astle & Hawkins. See Project 
//report for full reference

bool exiting = false;
long windowWidth = 1024;
long windowHeight = 600;
long windowBits = 24;
const char APP_NAME[] = "Managed Demo";
HDC hDC;

// global pointer to Demo class
Game* g_Game;

void SetupPixelFormat(HDC hDC)
{
	int pixelFormat;

	PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size
		1,								// version
		PFD_SUPPORT_OPENGL |			// OpenGL window
		PFD_DRAW_TO_WINDOW |			// render to window
		PFD_DOUBLEBUFFER   |			// support double-buffering
		PFD_TYPE_RGBA,	   				// colour type
		32,								// prefered colour depth
		0, 0, 0, 0, 0, 0,				// colour bits (ignored)
		0,								// no alpha buffer
		0,								// alpha bits (ignored)
		0,								// no accumulation buffer
		0, 0, 0, 0,						// accum bits (ignored)
		16,								// depth buffer
		0,								// no stencil buffer
		0,								// no auxilary buffers
		PFD_MAIN_PLANE,					// main layer
		0,								// reserved
		0, 0, 0,						// no layer, visible, damage masks
	};

	pixelFormat = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, pixelFormat, &pfd);
}


LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HDC hDC;
	static HGLRC hRC;
	int height, width;

	// dispatch messages
	switch (uMsg)
	{
	case WM_CREATE:					// window creation
		hDC = GetDC(hWnd);
		SetupPixelFormat(hDC);
		hRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hRC);
		break;
	case WM_DESTROY:				// window destroy
	case WM_QUIT:
	case WM_CLOSE:					// windows is closing

		// deselect rendering context and delete it
		wglMakeCurrent(hDC, NULL);
		wglDeleteContext(hRC);

		// send WM_QUIT to message queue
		PostQuitMessage(0);
		break;

	case WM_SIZE:
		height = HIWORD(lParam);	// retrieve width and height
		width = LOWORD(lParam);

		g_Game->Resize(width, height);

		break;

	case  WM_KEYDOWN:
		int fwKeys;
		LPARAM keyData;
		fwKeys = (int)wParam;		// virtual-key code
		keyData = lParam;			// key data

		switch (fwKeys)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void EnableCrashingOnCrashes() 
{ 
	typedef BOOL (WINAPI *tGetPolicy)(LPDWORD lpFlags); 
	typedef BOOL (WINAPI *tSetPolicy)(DWORD dwFlags); 
	const DWORD EXCEPTION_SWALLOWING = 0x1;

	HMODULE kernel32 = LoadLibraryA("kernel32.dll"); 
	tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32, 
		"GetProcessUserModeExceptionPolicy"); 
	tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32, 
		"SetProcessUserModeExceptionPolicy"); 
	if (pGetPolicy && pSetPolicy) 
	{ 
		DWORD dwFlags; 
		if (pGetPolicy(&dwFlags)) 
		{ 
			// Turn off the filter 
			pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING); 
		} 
	} 
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX windowClass;				// window class
	HWND hwnd;							// window handle
	MSG msg;							// message
	DWORD dwExStyle;					// Window Extended Style
	DWORD dwStyle;						// Window Style
	RECT windowRect;

	windowRect.left = (long)0;				// Set left value to 0
	windowRect.right = (long)windowWidth;	// Set right value to requested width
	windowRect.top = (long)0;				// Set top value to 0
	windowRect.bottom = (long)windowHeight; // Set bottom value to requeted height

	// fill out the window class structure
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = MainWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	// Default icon
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);		// Default arrow
	windowClass.hbrBackground = NULL;						// don't need a background
	windowClass.lpszMenuName = NULL;						// don't need a menu
	windowClass.lpszClassName = L"GLClass";
	windowClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);		// windows small logo icon

	// register the window class
	if (!RegisterClassEx(&windowClass))
		return 0;

	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;		// Window extended style
	dwStyle = WS_OVERLAPPEDWINDOW;						// Window style


	// Adjust window to true requested size
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	// class registered, so now create our window
	hwnd = CreateWindowEx(	NULL,								// extended style
		L"GLClass",							// class name
		L"Managed Demo",				// app name
		dwStyle | WS_CLIPCHILDREN | 
		WS_CLIPSIBLINGS,
		0, 0,  								// x, y coords
		windowRect.right - windowRect.left, // width
		windowRect.bottom - windowRect.top, // height
		NULL,								//handle to parent
		NULL,								// handle to menu
		hInstance,							// application instance
		NULL);								// no extra params


	hDC = GetDC(hwnd);

	EnableCrashingOnCrashes();

	// check if window creation failed (hwnd would equal NULL)
	if (!hwnd)
		return 0;

	g_Game = new Demo(hDC);

	ShowWindow(hwnd, SW_SHOW);				// display the window
	UpdateWindow(hwnd);						// update the window

	while (!exiting)
	{
		exiting = g_Game->Run();

		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
			{
				exiting = true;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	delete g_Game;
	g_Game = NULL;

	return (int)msg.wParam;
}