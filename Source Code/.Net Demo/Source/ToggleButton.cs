using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace DotNetDemo
{
    public class ToggleButton : DemoButton
    {
        public ToggleButton(Rectangle rect, Color col)
            : base(rect, col)
        {
            isPressed = false;
        }

        public override void Depress()
        {
            isPressed = !isPressed;
            if (isPressed)
            {
                demo.selectDestroyer = true;
            }
            else
            {
                demo.selectDestroyer = false;
            }
        }
    }
}
