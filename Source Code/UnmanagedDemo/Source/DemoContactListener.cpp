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
		GameObject* gameObjA = (GameObject*)bodyA->GetUserData();
		GameObject* gameObjB = (GameObject*)bodyB->GetUserData();
		if (gameObjA->destroyer)
		{
			m_demo->toDestroy.push_back(gameObjB);
		}
		else if (gameObjB->destroyer)
		{
			m_demo->toDestroy.push_back(gameObjA);
		}
	}
}

DemoContactListener::DemoContactListener( Demo* demo )
{
	m_demo = demo;
}
