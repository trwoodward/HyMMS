using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace DotNetDemo
{
    public class DemoButton
    {
        public Rectangle bounds;
        public Texture2D releasedTex;
        public Texture2D pressedTex;
        public Color col;
        public CSharpDemo demo;
        public bool isPressed;

        public DemoButton() { }
        public DemoButton(Rectangle rect, Color _col)
        {
            bounds = rect;
            col = _col;
        }

        public virtual void Depress() { }
        public virtual void Unpress() { }

        public bool IsInBounds(float x, float y)
        {
            if (x < bounds.X || (x > bounds.X + bounds.Width) || y < bounds.Y || y > (bounds.Y + bounds.Height))
                return false;
            return true;
        }
    }
}
