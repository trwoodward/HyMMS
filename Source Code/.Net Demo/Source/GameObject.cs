using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Box2D.XNA;

namespace DotNetDemo
{
    public class GameObject
    {
        public Vector2 scale;
        public Vector2 position;
        public Vector2 origin;
        public float rotation;
        public Texture2D texture;
        public Rectangle srcRect;
        public Color tint;
        public bool destroyer;
        public int index;
        public Body physBody;

        public GameObject()
        {
            position = Vector2.Zero;
            origin = Vector2.Zero;
            scale = Vector2.One;
            rotation = 0.0f;
            texture = null;
            srcRect = new Rectangle();
            tint = new Color();
            destroyer = false;
            index = -1;
        }

        public float Width
        {
            get { return (scale.X * texture.Width); }
            set { scale.X = (value / texture.Width); }
        }

        public float Height
        {
            get { return (scale.Y * texture.Height); }
            set { scale.Y = (value / texture.Height); }
        }


    }
}
