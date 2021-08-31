#pragma once
#include <Box2D/Box2D.h>
#include "Demo.h"

class DemoContactListener :
	public b2ContactListener
{
public:
	DemoContactListener(Demo* demo);
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);

private:
	Demo* m_demo;
};

