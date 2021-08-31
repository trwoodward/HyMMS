#include "stdafx.h"

#ifdef _WINDOWS_
#include <windows.h>
#endif // _WINDOWS_

#include <gl/GL.h>
#include <gl/GLU.h>

#include "Game.h"
#include "ContentManager.h"


Game::Game(HDC deviceContext)
{
	hDC = deviceContext;
	firstTime = true;
	exiting = false;
}

Game::~Game(void)
{
}

bool Game::Run()
{
	if (firstTime)
	{
		Initialize();
		LoadContent();
		firstTime = false;
	}
	
	static double prevTime = GCTimer::CurrentTimeInMS();
	//Get time...
	double currTime = GCTimer::CurrentTimeInMS(); 
	double elapsedTime = currTime - prevTime;
	prevTime = currTime;
	Update(elapsedTime);
	Draw(elapsedTime);

	//Return if we're exiting
	return exiting;
}

void Game::Update( double elapsedTimeInMS )
{
}

void Game::Draw( double elapsedTimeInMS )
{
	SwapBuffers(hDC);
}

void Game::Initialize()
{
	glClearColor(0.75, 0.0, 1.0, 0.0);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_POLYGON_SMOOTH);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	contentManager = new ContentManager();
}

void Game::LoadContent()
{
}

void Game::Resize( int width, int height )
{
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set up projection of window
	glOrtho(0.0, (GLfloat)width, (GLfloat)height, 0.0, 0.0, -1.0);

	glMatrixMode(GL_MODELVIEW); // set modelview matrix
	glLoadIdentity(); // reset modelview matrix

	windowWidth = width;
	windowHeight = height;
}

void Game::Exit()
{
	exiting = true;
}
