#pragma once
#ifdef  _WINDOWS_
#include <windows.h>
#endif //  _WINDOWS_

#include <gl/GL.h>
#include <gl/GLU.h>

#include "GCTimer.h"
#include "ContentManager.h"
#include "DemoDebugInfo.h"
#include "PerfInfo.h"

class Game
{
public:
	Game(HDC deviceContext);
	virtual ~Game();

	bool Run();
	virtual void Update(double elapsedTimeInMS) = 0;
	virtual void Draw(double elapsedTimeInMS) = 0;
	virtual void Initialize() = 0;
	virtual void LoadContent() = 0;
	void Resize(int width, int height);
	void Exit();
private:
	bool firstTime;
	bool exiting;

protected:
	int windowHeight;
	int windowWidth;
	SmartHandle<ContentManager> contentManager;
	HDC hDC;

	//DemoDebugInfo info;

	PerfInfo* info;
	bool recording;
	double lastFrameTime;
	int numCols;
};

