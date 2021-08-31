using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Box2D.XNA;

namespace DotNetDemo
{
    public class QueryCallback
    {
        Vector2 m_point;
        public Fixture m_fixture;

        public QueryCallback(Vector2 point)
        {
            m_point = point;
            m_fixture = null;
        }

        public bool ReportFixture(FixtureProxy fixtureProxy)
        {
            Fixture fixture = fixtureProxy.fixture;
            Body body = fixture.GetBody();
            if (body.GetType() == BodyType.Dynamic)
            {
                bool inside = fixture.TestPoint(m_point);
                if (inside)
                {
                    m_fixture = fixture;

                    return false;
                }
            }

            return true;
        }
    }
}
