using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Box2D.XNA;

namespace DotNetDemo
{
    public class DemoContactListener : IContactListener
    {
        CSharpDemo m_demo;

        public DemoContactListener(CSharpDemo demo)
        {
            m_demo = demo;
        }

        public void BeginContact(Contact contact) { }
        public void EndContact(Contact contact) { }


        public void PreSolve(Contact contact, ref Manifold oldManifold) 
        {
            WorldManifold worldManifold;

            contact.GetWorldManifold(out worldManifold);

            Body bodyA = contact.GetFixtureA().GetBody();
            Body bodyB = contact.GetFixtureB().GetBody();

            if (worldManifold._normal.Y < -0.5f)
            {
                if (bodyA == m_demo.topWall || bodyB == m_demo.topWall)
                {
                    contact.SetEnabled(false);
                }
            }

            if (bodyA.GetType() == BodyType.Dynamic && bodyB.GetType() == BodyType.Dynamic)
            {
                int gameObjAIndx = (int)bodyA.GetUserData();
                int gameObjBIndx = (int)bodyB.GetUserData();
                if (m_demo.objects[gameObjAIndx].destroyer)
                {
                    m_demo.toDestroy.Add(gameObjBIndx);
                }
                else if (m_demo.objects[gameObjBIndx].destroyer)
                {
                    m_demo.toDestroy.Add(gameObjAIndx);
                }
            }
        }


        public void PostSolve(Contact contact, ref ContactImpulse impulse) 
        {
        }
    }
}
