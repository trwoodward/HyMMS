#include "stdafx.h"
#include "DemoContactListener.h"

//Based on code from the Box2D manual on Box2D.org
void DemoContactListener::PreSolve( b2Contact* contact, const b2Manifold* oldManifold )
{
	b2WorldManifold worldManifold;

	contact->GetWorldManifold(&worldManifold);

	b2Body* bodyA = contact->GetFixtureA()->GetBody();
	b2Body* bodyB = contact->GetFixtureB()->GetBody();

	if (worldManifold.normal.y < -0.5f)
	{
		if (bodyA == m_demo->topWall || bodyB == m_demo->topWall)
		{
			contact->SetEnabled(false);
		}
	}

	if (bodyA->GetType() == b2_dynamicBody && bodyB->GetType() == b2_dynamicBody)
	{
		int gameObjAIndx = (int)bodyA->GetUserData();
		int gameObjBIndx = (int)bodyB->GetUserData();
		if (m_demo->objects[gameObjAIndx]->destroyer)
		{
			m_demo->toDestroy.push_back(gameObjBIndx);
		}
		else if (m_demo->objects[gameObjBIndx]->destroyer)
		{
			m_demo->toDestroy.push_back(gameObjAIndx);
		}
	}
}

DemoContactListener::DemoContactListener( Demo* demo )
{
	m_demo = demo;
}
