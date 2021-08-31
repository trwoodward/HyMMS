#include "stdafx.h"
#include "Demo.h"
#include "DemoRectangle.h"
#include "DemoColour.h"
#include "QueryCallback.h"
#include "VSyncHelper.h"
#define PI 3.14159f


Demo::Demo(HDC deviceContext) : Game(deviceContext), mPerPix(0.0651f), pixPerM(15.36f)//, info(deviceContext)
{
	ang = 0.0f;
	numLiveObjects = 0;
	maxObjects = 50;

	prevLeftButtonDown = false;
	m_mouseJoint = NULL;
	listener = new DemoContactListener(this);
	destroyer = NULL;
	selectDestroyer = false;
	toDestroy = std::vector<int>();
	setVSync(0);
	
}


Demo::~Demo(void)
{
	delete listener;
}

void Demo::Initialize()
{
	//Initialize managed components
	objects = gcnew(Handle_Array<GameObject>)(maxObjects);
	for (int i = 0; i < maxObjects; ++i)
	{
		objects[i] = NULL;
	}
	sBatch = gcnew(SpriteBatch)();
	buttons = gcnew(Handle_Array<DemoButton>)(2);

	input = new WindowsInputManager(WindowFromDC(hDC));

	//Box2D initialization
	b2Vec2 gravity(0.0f, 20.0f);
	physWorld = new b2World(gravity);

	b2BodyDef boxBodydef;
	boxBodydef.type = b2_staticBody;
	b2Vec2 boxPos = ToWorld(130.0f, (float)windowHeight / 2.0f);
	boxBodydef.position.Set(boxPos.x, boxPos.y);
	leftWall = physWorld->CreateBody(&boxBodydef);

	b2Vec2 verticalWallDims = ToWorld(10.0f, (float)windowHeight / 2.0f);
	b2PolygonShape verticalWall;
	verticalWall.SetAsBox(verticalWallDims.x, verticalWallDims.y);

	b2FixtureDef wallFixtureDef;
	wallFixtureDef.shape = &verticalWall;
	wallFixtureDef.friction = 0.3f;

	leftWall->CreateFixture(&wallFixtureDef);

	boxPos = ToWorld((float)windowWidth, (float)windowHeight / 2.0f);
	boxBodydef.position.Set(boxPos.x, boxPos.y);
	rightWall = physWorld->CreateBody(&boxBodydef);

	rightWall->CreateFixture(&wallFixtureDef);

	boxPos = ToWorld(((float)windowWidth / 2.0f) + 130.0f, 0.0f);
	boxBodydef.position.Set(boxPos.x, boxPos.y);
	topWall = physWorld->CreateBody(&boxBodydef);

	b2Vec2 horizontalWallDims = ToWorld((float)windowWidth / 2.0f, 10.0f);
	b2PolygonShape horizontalWall;
	horizontalWall.SetAsBox(horizontalWallDims.x, horizontalWallDims.y);
	wallFixtureDef.shape = &horizontalWall;
	topWall->CreateFixture(&wallFixtureDef);

	physWorld->SetContactListener(listener);

	boxPos = ToWorld(((float)windowWidth / 2.0f) + 130.0f, (float)windowHeight);
	boxBodydef.position.Set(boxPos.x, boxPos.y);
	bottomWall = physWorld->CreateBody(&boxBodydef);

	bottomWall->CreateFixture(&wallFixtureDef);

	//Button creation
	buttons[0] = gcnew(PushButton)(DemoRectangle(0, 0, 130, 75), NULL, DemoColour(1.0f, 1.0f, 1.0f, 1.0f));;
	buttons[1] = gcnew(ToggleButton)(DemoRectangle(0, 150, 130, 75), NULL, DemoColour(1.0f, 1.0f, 1.0f, 1.0f));

	buttons[0]->demo = this;
	buttons[1]->demo = this;

	boundary = DemoRectangle(130, 0, windowWidth - 130, windowHeight);

	//Performance metric gathering only
	/*
	info = new PerfInfo();
	printed = false;
	recording = false;
	lastFrameTime = 0.0;
	numCols = 0;*/

	Game::Initialize();
}

void Demo::LoadContent()
{
	crateTex = contentManager->Load<Texture2D>("BoxTex.tga");
	boundaryTex = contentManager->Load<Texture2D>("BoundaryTex.tga");
	spawnButtonReleasedTex = contentManager->Load<Texture2D>("SpawnButtonReleased.tga");
	spawnButtonPressedTex = contentManager->Load<Texture2D>("SpawnButtonPressed.tga");
	destroyButtonReleasedTex = contentManager->Load<Texture2D>("DestroyButtonReleased.tga");
	destroyButtonPressedTex = contentManager->Load<Texture2D>("DestroyButtonPressed.tga");


	buttons[0]->releasedTex = spawnButtonReleasedTex;
	buttons[0]->pressedTex = spawnButtonPressedTex;
	buttons[1]->releasedTex = destroyButtonReleasedTex;
	buttons[1]->pressedTex = destroyButtonPressedTex;

	Game::LoadContent();
}

void Demo::Update( double elapsedTime )
{
	if (input->IsKeyDown('P'))
	{
		if (!printed)
		{
			info->StopAndPrint();
			printed = true;
			recording = false;
		}
	}

	//Used for performance metric recording
	/*if (recording)
	{
		double collTime = GCTimer::CurrentTimeInMS() - lastFrameTime;
		info->Tick((double)collTime);
	}*/

	//Used for performance metric recording
	/*if (recording)
	{
		if (info->timeSoFar >= 30000.0)
		{
			info->StopAndPrint();
			recording = false;
		}
	}*/

	if (input->IsKeyDown('B'))
	{
		recording = true;
	}

	for (unsigned int i = 0; i < toDestroy.size(); ++i)
	{
		physWorld->DestroyBody(objects[toDestroy[i]]->physBody);
		objects[toDestroy[i]] = NULL;
	}
	toDestroy.clear();

	input->GetMousePosition(mouseX, mouseY);
	bool leftButtonDown = input->IsLeftMouseButtonDown();
	b2Vec2 mPos = ToWorld(mouseX, mouseY);
	if (leftButtonDown)
	{
		if (prevLeftButtonDown)
		{
			MouseMove(mPos);
		}
		else
		{
			MouseDown(mPos);
		}
	}
	else
	{
		if (prevLeftButtonDown)
		{
			MouseUp(mPos);
		}
	}
	prevLeftButtonDown = leftButtonDown;

	b2Vec2 bWorldPos;
	b2Vec2 bScreenPos;

	for (int i = 0; i < maxObjects; ++i)
	{
		if (objects[i] != NULL)
		{
			bWorldPos = objects[i]->physBody->GetPosition();
			bScreenPos = ToScreen(bWorldPos.x, bWorldPos.y);
			objects[i]->position[0] = bScreenPos.x;
			objects[i]->position[1] = bScreenPos.y;
			objects[i]->rotation = objects[i]->physBody->GetAngle();
		}
	}

	physWorld->Step(elapsedTime / 1000.0f, 10, 10);
	//Used for performance metric recording
	/*if (recording)
	{
		double physTime = GCTimer::CurrentTimeInMS() - lastFrameTime;
		info->Tick((double)physTime);
	}*/

	Game::Update(elapsedTime);
}

void Demo::Draw( double elapsedTimeInMS )
{
	glClearColor(0.39f, 0.58f, 0.93f, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	sBatch->Begin();
	sBatch->Draw(boundaryTex, boundary, DemoColour(1.0f, 1.0f, 1.0f, 1.0f));
	for (int i = 0; i < maxObjects; ++i)
	{
		if (objects[i] != NULL)
			sBatch->Draw(objects[i]->texture, objects[i]->position, NULL, objects[i]->tint, objects[i]->rotation, objects[i]->origin, objects[i]->scale);
	}
	for (int i = 0; i < 2; ++i)
	{
		SmartHandle<Texture2D> buttonTex = (buttons[i]->isPressed) ? buttons[i]->pressedTex : buttons[i]->releasedTex;
		sBatch->Draw(buttonTex, buttons[i]->bounds, buttons[i]->col);
	}
	sBatch->End();
	//info.DrawInfoToScreen(); //Used for demonstration to supervisor
	//info.UpdateFrameRate(elapsedTimeInMS); //Used for demonstration to supervisor
	Game::Draw(elapsedTimeInMS);
	//Used for demonstration to supervisor
	/*if (recording)
	{
		lastFrameTime = GCTimer::CurrentTimeInMS();
	}*/
}

bool Demo::AddGameObject( SmartHandle<GameObject> obj )
{
	for (int i = 0; i < maxObjects; ++i)
	{
		if (objects[i] == NULL)
		{
			objects[i] = obj;
			obj->index = i;
			++numLiveObjects;
			return true;
		}
	}
	return false;
}

b2Vec2 Demo::ToWorld( float screenCoordX, float screenCoordY )
{
	b2Vec2 retVec;
	retVec.x = screenCoordX * mPerPix;
	retVec.y = screenCoordY * mPerPix;
	return retVec;
}

b2Vec2 Demo::ToScreen( float worldCoordX, float worldCoordY )
{
	b2Vec2 retVec;
	retVec.x = worldCoordX * pixPerM;
	retVec.y = worldCoordY * pixPerM;
	return retVec;
}

void Demo::MouseDown( const b2Vec2& pos )
{
	b2Vec2 screenPos = ToScreen(pos.x, pos.y);
	for (int i = 0; i < 2; ++i)
	{
		if (buttons[i]->IsInBounds(screenPos.x, screenPos.y))
		{
			buttons[i]->Depress();
			return;
		}
	}

	if (m_mouseJoint != NULL)
	{
		return;
	}

	//Make a small AABB
	b2AABB aabb;
	b2Vec2 delta;
	delta.Set(0.001f, 0.001f);
	aabb.lowerBound = pos - delta;
	aabb.upperBound = pos + delta;

	//Query world for overlapping shapes
	QueryCallback callback(pos);
	physWorld->QueryAABB(&callback, aabb);

	if (callback.m_fixture)
	{
		b2Body* body = callback.m_fixture->GetBody();
		b2MouseJointDef md;
		md.bodyA = bottomWall;
		md.bodyB = body;
		md.target = pos;
		md.maxForce = 1000.0f * body->GetMass();
		m_mouseJoint = (b2MouseJoint*)physWorld->CreateJoint(&md);
		body->SetAwake(true);
		if (selectDestroyer)
		{
			int objIndx = (int)body->GetUserData();
			SmartHandle<GameObject> obj = objects[objIndx];
			obj->destroyer = true;
			destroyer = obj;
		}
	}
}

void Demo::MouseUp( const b2Vec2& pos )
{
	for (int i = 0; i < 2; ++i)
	{
		if (buttons[i]->isPressed)
		{
			buttons[i]->Unpress();
		}
	}

	if (m_mouseJoint)
	{
		physWorld->DestroyJoint(m_mouseJoint);
		m_mouseJoint = NULL;
		if (destroyer != SmartHandle<GameObject>())
		{
			destroyer->destroyer = false;
			destroyer = NULL;
		}
	}
}

void Demo::MouseMove( const b2Vec2& pos )
{
	if (m_mouseJoint)
	{
		m_mouseJoint->SetTarget(pos);
	}
}

void Demo::SpawnObject()
{
	SmartHandle<GameObject> obj = gcnew(GameObject)();
	double randomWeight = DemoRand::GetRandNormalized();
	obj->position[0] = 250.0f + (randomWeight * ((double)windowWidth - 350.0f));
	obj->position[1] = -75.0f;
	obj->Width() = 50.0f;
	obj->Height() = 50.0f;

	obj->origin[0] = 0.5f;
	obj->origin[1] = 0.5f;

	obj->tint.R() = 1.0f;
	obj->tint.G() = 1.0f;
	obj->tint.B() = 1.0f;
	obj->tint.A() = 1.0f;

	b2BodyDef mould;
	mould.type = b2_dynamicBody;
	b2Vec2 newPos = ToWorld(obj->position[0], obj->position[1]);
	mould.position.Set(newPos.x, newPos.y);
	obj->physBody = physWorld->CreateBody(&mould);

	b2PolygonShape physBox;
	physBox.SetAsBox(ToWorld(25.0f, 0).x, ToWorld(25.0f, 0).x);

	b2FixtureDef boxFixture;
	boxFixture.shape = &physBox;

	boxFixture.density = 1.0f;
	boxFixture.friction = 0.3f;

	obj->physBody->CreateFixture(&boxFixture);
	obj->texture = crateTex;

	AddGameObject(obj);
	obj->physBody->SetUserData((void*)obj->index);
}
