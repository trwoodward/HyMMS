using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace DotNetDemo
{
    public class PushButton : DemoButton
    {
        public PushButton(Rectangle rect, Color col)
            : base(rect, col)
        {
            isPressed = false;
        }

        public override void Depress()
        {
            isPressed = true;
            demo.SpawnObject();
        }

        public override void Unpress()
        {
            isPressed = false;
        }
    }
}
