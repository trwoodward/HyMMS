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

	bool AddGameObject(GameObject* obj);
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

	//Unmanaged resources
	SpriteBatch sBatch;
	Texture2D* crateTex;
	Texture2D* boundaryTex;
	Texture2D* spawnButtonReleasedTex;
	Texture2D* spawnButtonPressedTex;
	Texture2D* destroyButtonPressedTex;
	Texture2D* destroyButtonReleasedTex;

	float ang;
	GameObject** objects;
	int numLiveObjects;
	int maxObjects;

	//Box2D members
	b2World* physWorld;
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

	DemoButton* buttons[2];

	GameObject* destroyer;
	bool selectDestroyer;
	std::vector<GameObject*> toDestroy;

	DemoRectangle boundary;

	const float mPerPix;
	const float pixPerM;

	//Performance metric use only
	/*PerfInfo* info;
	bool printed;
	bool recording;
	double lastFrameTime;*/
};

#endif

