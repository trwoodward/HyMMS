#include "stdafx.h"

#ifdef _WINDOWS_
#include <windows.h>
#endif // _WINDOWS_

#include <gl/GL.h>
#include <gl/GLU.h>

#include "Game.h"
#include "ContentManager.h"


Game::Game(HDC deviceContext)// : info(deviceContext)
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
		GC::CreateHeap(1024 * 1024);
		GC::SetLargeObjThreshold(1024);
		GC::SetCollectorBudget(2.0);
		Initialize();
		LoadContent();
		firstTime = false;
		GC::CollectAll();
	}
	
	static double prevTime = GCTimer::CurrentTimeInMS();
	//Get time...
	double currTime = GCTimer::CurrentTimeInMS(); 
	double elapsedTime = currTime - prevTime;
	prevTime = currTime;
	Update(elapsedTime);
	Draw(elapsedTime);

	//Used for performance metric measuring only
	/*lastFrameTime = 0.0;
	if (recording)
	{
		lastFrameTime = GCTimer::CurrentTimeInMS();
	}*/
	GC::IncCollect();
	//Used for performance metric measuring only
	/*
	if (recording)
	{
		//Used for performance metric measuring only
		//double collTime = GCTimer::CurrentTimeInMS() - lastFrameTime;
		//info->Tick((double)collTime);
		int temp = GC::GetNumCols();
		if (temp > 0 && temp != numCols)
		{
			info->Tick(GC::GetLastNurCollectionTime());
			numCols = temp;
		}
	}
	*/

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

	contentManager = gcnew(ContentManager)();
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
