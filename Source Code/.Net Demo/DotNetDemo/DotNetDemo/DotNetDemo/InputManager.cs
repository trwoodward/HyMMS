using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;

namespace DotNetDemo
{
    public class WindowsInputManager
    {
        public void GetMousePosition(out float x, out float y)
        {
            MouseState state = Mouse.GetState();
            x = state.X;
            y = state.Y;
        }

        public bool IsLeftMouseButtonDown()
        {
            MouseState state = Mouse.GetState();
            return (state.LeftButton == ButtonState.Pressed);
        }
    }
}
