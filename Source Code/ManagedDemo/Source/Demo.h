#pragma once
#ifdef _WINDOWS_
#include <windows.h>
#endif

#ifndef _DEMO_H_
#define _DEMO_H_

#include "game.h"
#include "SpriteBatch.h"
#include "GameObject.h"
#include "WindowsInputManager.h"
#include "DemoButton.h"
#include "PushButton.h"
#include "ToggleButton.h"
#include "DemoContactListener.h"
#include "DemoRand.h"
#include "DemoDebugInfo.h"
#include <vector>
#include "PerfInfo.h"


#include <gl/GL.h>
#include <gl/GLU.h>
#include "Texture2D.h"
#include <Box2D/Box2D.h>

class Demo :
	public Game
{
public:
	Demo(HDC deviceContext);
	~Demo(void);

	void Initialize();
	void LoadContent();
	void Update(double elapsedTime);
	void Draw(double elapsedTimeInMS);

	bool AddGameObject(SmartHandle<GameObject> obj);
	void SpawnObject();

private:
	friend class DemoContactListener;
	friend class ToggleButton;

	//Helper functions for interaction with Box2D library
	b2Vec2 ToWorld(float screenCoordX, float screenCoordY);
	b2Vec2 ToScreen(float worldCoordX, float worldCoordY);

	//Mouse event functions
	void MouseDown(const b2Vec2& pos);
	void MouseUp(const b2Vec2& pos);
	void MouseMove(const b2Vec2& pos);

	//Managed resources
	SmartHandle<SpriteBatch> sBatch;

	SmartHandle<Texture2D> crateTex;
	SmartHandle<Texture2D> boundaryTex;
	SmartHandle<Texture2D> spawnButtonReleasedTex;
	SmartHandle<Texture2D> spawnButtonPressedTex;
	SmartHandle<Texture2D> destroyButtonPressedTex;
	SmartHandle<Texture2D> destroyButtonReleasedTex;

	HndlArrHandle<GameObject> objects;

	HndlArrHandle<DemoButton> buttons;
	SmartHandle<GameObject> destroyer;

	float ang;
	
	int numLiveObjects;
	int maxObjects;

	b2World* physWorld;
	
	//Box2D members
	b2Body* bottomWall;
	b2Body* rightWall;
	b2Body* leftWall;
	b2Body* topWall;
	DemoContactListener* listener;
	b2MouseJoint* m_mouseJoint;

	//Input state
	WindowsInputManager* input;
	float mouseX;
	float mouseY;
	bool prevLeftButtonDown;

	bool selectDestroyer;
	std::vector<int> toDestroy;

	DemoRectangle boundary;

	const float mPerPix;
	const float pixPerM;

	//PerfInfo* info; //Performance metric use only
	bool printed;
	//bool recording; //Performance metric use only
	//double lastFrameTime; //Performance metric use only
};

#endif

