using System;
using System.Collections.Generic;
using System.Linq;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.GamerServices;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Media;
using Box2D.XNA;
using System.Diagnostics;

namespace DotNetDemo
{
    /// <summary>
    /// This is the main type for your game
    /// </summary>
    public class CSharpDemo : Microsoft.Xna.Framework.Game
    {
        GraphicsDeviceManager graphics;
        SpriteBatch spriteBatch;
        MouseJoint m_mouseJoint = null;

        int numLiveObjects = 0;
        int maxObjects = 50;

        bool prevLeftButtonDown = false;
        DemoContactListener listener;
        GameObject destroyer;

        public bool selectDestroyer = false;
        public List<int> toDestroy;
        public List<GameObject> objects;

        List<DemoButton> buttons;
        World physWorld;

        const float mPerPix = 0.0651f;
        const float pixPerM = 15.36f;

        public Body leftWall;
        public Body rightWall;
        public Body topWall;
        public Body bottomWall;

        Random rand;
        Texture2D crateTex;
        Texture2D boundaryTex;
        Texture2D spawnButtonReleasedTex;
        Texture2D spawnButtonPressedTex;
        Texture2D destroyButtonPressedTex;
        Texture2D destroyButtonReleasedTex;

        Rectangle boundary;

        WindowsInputManager input;
        float mouseX = 0.0f;
        float mouseY = 0.0f;

        PerfInfo info;
        Stopwatch phyTimer;
        bool printed = false;
        bool recording = false;

        public CSharpDemo()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";

            numLiveObjects = 0;
            maxObjects = 50;

            prevLeftButtonDown = false;
            m_mouseJoint = null;
            listener = new DemoContactListener(this);
            destroyer = null;
            selectDestroyer = false;
            toDestroy = new List<int>();
            rand = new Random();
            IsMouseVisible = true;
            graphics.PreferredBackBufferWidth = 1024;
            graphics.PreferredBackBufferHeight = 600;

            IsFixedTimeStep = false;
            graphics.SynchronizeWithVerticalRetrace = false;
            graphics.ApplyChanges();
        }

        /// <summary>
        /// Allows the game to perform any initialization it needs to before starting to run.
        /// This is where it can query for any required services and load any non-graphic
        /// related content.  Calling base.Initialize will enumerate through any components
        /// and initialize them as well.
        /// </summary>
        protected override void Initialize()
        {
            // TODO: Add your initialization logic here
            objects = new List<GameObject>(maxObjects);
            for (int i = 0; i < maxObjects; ++i)
            {
                objects.Add(null);
            }
            buttons = new List<DemoButton>(2);

            input = new WindowsInputManager();

            Vector2 gravity = new Vector2(0.0f, 20.0f);
            physWorld = new World(gravity, true);

            BodyDef boxBodyDef = new BodyDef();
            boxBodyDef.type = BodyType.Static;
            Vector2 boxPos = ToWorld(130.0f, (float)graphics.GraphicsDevice.Viewport.Height / 2.0f);
            boxBodyDef.position = boxPos;
            leftWall = physWorld.CreateBody(boxBodyDef);

            Vector2 verticalWallDims = ToWorld(10.0f, (float)graphics.GraphicsDevice.Viewport.Height / 2.0f);
            PolygonShape verticalWall = new PolygonShape();
            verticalWall.SetAsBox(verticalWallDims.X, verticalWallDims.Y);

            FixtureDef wallFixtureDef = new FixtureDef();
            wallFixtureDef.shape = verticalWall;
            wallFixtureDef.friction = 0.3f;

            leftWall.CreateFixture(wallFixtureDef);

            boxPos = ToWorld((float)graphics.GraphicsDevice.Viewport.Width, (float)graphics.GraphicsDevice.Viewport.Height / 2.0f);
            boxBodyDef.position = boxPos;
            rightWall = physWorld.CreateBody(boxBodyDef);

            rightWall.CreateFixture(wallFixtureDef);

            boxPos = ToWorld(((float)graphics.GraphicsDevice.Viewport.Width / 2.0f) + 130.0f, 0.0f);
            boxBodyDef.position = boxPos;
            topWall = physWorld.CreateBody(boxBodyDef);

            Vector2 horizontalWallDims = ToWorld((float)graphics.GraphicsDevice.Viewport.Width / 2.0f, 10.0f);
            PolygonShape horizontalWall = new PolygonShape();
            horizontalWall.SetAsBox(horizontalWallDims.X, horizontalWallDims.Y);
            wallFixtureDef.shape = horizontalWall;
            topWall.CreateFixture(wallFixtureDef);

            physWorld.ContactListener = listener;

            boxPos = ToWorld(((float)graphics.GraphicsDevice.Viewport.Width / 2.0f) + 130.0f, (float)graphics.GraphicsDevice.Viewport.Height);
            boxBodyDef.position = boxPos;
            bottomWall = physWorld.CreateBody(boxBodyDef);

            bottomWall.CreateFixture(wallFixtureDef);

            buttons.Add(new PushButton(new Rectangle(0, 0, 130, 75), Color.White));
            buttons.Add(new ToggleButton(new Rectangle(0, 150, 130, 75), Color.White));

            buttons[0].demo = this;
            buttons[1].demo = this;

            boundary = new Rectangle(130, 0, graphics.GraphicsDevice.Viewport.Width - 130, graphics.GraphicsDevice.Viewport.Height);

            info = new PerfInfo();
            phyTimer = new Stopwatch();

            base.Initialize();
        }

        /// <summary>
        /// LoadContent will be called once per game and is the place to load
        /// all of your content.
        /// </summary>
        protected override void LoadContent()
        {
            // Create a new SpriteBatch, which can be used to draw textures.
            spriteBatch = new SpriteBatch(GraphicsDevice);

            crateTex = Content.Load<Texture2D>("BoxTex");
            boundaryTex = Content.Load<Texture2D>("BoundaryTex");
            spawnButtonReleasedTex = Content.Load<Texture2D>("SpawnButtonReleased");
            spawnButtonPressedTex = Content.Load<Texture2D>("SpawnButtonPressed");
            destroyButtonReleasedTex = Content.Load<Texture2D>("DestroyButtonReleased");
            destroyButtonPressedTex = Content.Load<Texture2D>("DestroyButtonPressed");

            buttons[0].releasedTex = spawnButtonReleasedTex;
            buttons[0].pressedTex = spawnButtonPressedTex;
            buttons[1].releasedTex = destroyButtonReleasedTex;
            buttons[1].pressedTex = destroyButtonPressedTex;

            base.LoadContent();
        }

        /// <summary>
        /// UnloadContent will be called once per game and is the place to unload
        /// all content.
        /// </summary>
        protected override void UnloadContent()
        {
            // TODO: Unload any non ContentManager content here
        }

        /// <summary>
        /// Allows the game to run logic such as updating the world,
        /// checking for collisions, gathering input, and playing audio.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Update(GameTime gameTime)
        {
            if (Keyboard.GetState().IsKeyDown(Keys.P))
            {
                if (!printed)
                {
                    info.StopAndPrint();
                    printed = true;
                    recording = false;
                }
            }
            if (Keyboard.GetState().IsKeyDown(Keys.B))
            {
                recording = true;
            }

            for (int i = 0; i < toDestroy.Count; ++i)
            {
                physWorld.DestroyBody(objects[toDestroy[i]].physBody);
                objects[toDestroy[i]] = null;
            }
            toDestroy.Clear();

            input.GetMousePosition(out mouseX, out mouseY);
            bool leftButtonDown = input.IsLeftMouseButtonDown();
            Vector2 mPos = ToWorld(mouseX, mouseY);
            if (leftButtonDown)
            {
                if (prevLeftButtonDown)
                {
                    MouseMove(mPos);
                }
                else
                {
                    MouseDown(mPos);
                }
            }
            else
            {
                if (prevLeftButtonDown)
                {
                    MouseUp(mPos);
                }
            }
            prevLeftButtonDown = leftButtonDown;

            Vector2 bWorldPos = Vector2.Zero;
            Vector2 bScreenPos = Vector2.Zero;

            for (int i = 0; i < maxObjects; ++i)
            {
                if (objects[i] != null)
                {
                    bWorldPos = objects[i].physBody.GetPosition();
                    bScreenPos = ToScreen(bWorldPos.X, bWorldPos.Y);
                    objects[i].position = bScreenPos;
                    objects[i].rotation = objects[i].physBody.GetAngle();
                }
            }

            if (recording)
            {
                phyTimer.Restart();
            }
            physWorld.Step((float)gameTime.ElapsedGameTime.TotalMilliseconds / 1000.0f, 10, 10);
            if (recording)
            {
                phyTimer.Stop();
                info.Tick(((double)phyTimer.ElapsedTicks / (double)Stopwatch.Frequency) * 1000.0);
            }

            base.Update(gameTime);
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {
            GraphicsDevice.Clear(Color.CornflowerBlue);

            spriteBatch.Begin();
            spriteBatch.Draw(boundaryTex, boundary, Color.White);
            for (int i = 0; i < maxObjects; ++i)
            {
                if (objects[i] != null)
                {
                    spriteBatch.Draw(objects[i].texture, objects[i].position, null, objects[i].tint, objects[i].rotation, objects[i].origin, objects[i].scale, SpriteEffects.None, 0.0f);
                }
            }
            for (int i = 0; i < 2; ++i)
            {
                Texture2D buttonTex = (buttons[i].isPressed) ? buttons[i].pressedTex : buttons[i].releasedTex;
                spriteBatch.Draw(buttonTex, buttons[i].bounds, buttons[i].col);
            }
            spriteBatch.End();

            base.Draw(gameTime);
        }

        Vector2 ToWorld(float screenCoordX, float screenCoordY)
        {
            Vector2 retVec;
            retVec.X = screenCoordX * mPerPix;
            retVec.Y = screenCoordY * mPerPix;
            return retVec;
        }

        Vector2 ToScreen(float worldCoordX, float worldCoordY)
        {
            Vector2 retVec;
            retVec.X = worldCoordX * pixPerM;
            retVec.Y = worldCoordY * pixPerM;
            return retVec;
        }

        public void SpawnObject()
        {
            GameObject obj = new GameObject();
            double randomWeight = rand.NextDouble();
            obj.position.X = (float)(250.0f + (randomWeight * ((double)graphics.GraphicsDevice.Viewport.Width - 350.0f)));
            obj.position.Y = -75.0f;

            obj.tint = Color.White;

            BodyDef mould = new BodyDef();
            mould.type = BodyType.Dynamic;
            Vector2 newPos = ToWorld(obj.position.X, obj.position.Y);
            mould.position = newPos;
            obj.physBody = physWorld.CreateBody(mould);

            PolygonShape physBox = new PolygonShape();
            physBox.SetAsBox(ToWorld(25.0f, 0).X, ToWorld(25.0f, 0).X);

            FixtureDef boxFixture = new FixtureDef();
            boxFixture.shape = physBox;

            boxFixture.density = 1.0f;
            boxFixture.friction = 0.3f;

            obj.physBody.CreateFixture(boxFixture);
            obj.texture = crateTex;
            obj.Width = 50.0f;
            obj.Height = 50.0f;

            obj.origin.X = 0.5f * crateTex.Width;
            obj.origin.Y = 0.5f * crateTex.Height;

            AddGameObj(obj);
            obj.physBody.SetUserData(obj.index);
        }

        public bool AddGameObj(GameObject obj)
        {
            for (int i = 0; i < maxObjects; ++i)
            {
                if (objects[i] == null)
                {
                    objects[i] = obj;
                    obj.index = i;
                    ++numLiveObjects;
                    return true;
                }
            }
            return false;
        }

        public void MouseDown(Vector2 pos)
        {
            Vector2 screenPos = ToScreen(pos.X, pos.Y);
            for (int i = 0; i < 2; ++i)
            {
                if (buttons[i].IsInBounds(screenPos.X, screenPos.Y))
                {
                    buttons[i].Depress();
                    return;
                }
            }

            if (m_mouseJoint != null)
            {
                return;
            }

            //Make a small AABB
            AABB aabb = new AABB();
            Vector2 delta = new Vector2(0.001f, 0.001f);
            aabb.lowerBound = pos - delta;
            aabb.upperBound = pos + delta;

            //Query world for overlapping shapes
            QueryCallback callBack = new QueryCallback(pos);
            physWorld.QueryAABB(callBack.ReportFixture, ref aabb);

            if (callBack.m_fixture != null)
            {
                Body body = callBack.m_fixture.GetBody();
                MouseJointDef md = new MouseJointDef();
                md.bodyA = bottomWall;
                md.bodyB = body;
                md.target = pos;
                md.maxForce = 1000.0f * body.GetMass();
                m_mouseJoint = (MouseJoint)physWorld.CreateJoint(md);
                body.SetAwake(true);
                if (selectDestroyer)
                {
                    int objIndx = (int)body.GetUserData();
                    GameObject obj = objects[objIndx];
                    obj.destroyer = true;
                    destroyer = obj;
                }
            }
        }

        public void MouseUp(Vector2 pos)
        {
            for (int i = 0; i < 2; ++i)
            {
                if (buttons[i].isPressed)
                {
                    buttons[i].Unpress();
                }
            }

            if (m_mouseJoint != null)
            {
                physWorld.DestroyJoint(m_mouseJoint);
                m_mouseJoint = null;
                if (destroyer != null)
                {
                    destroyer.destroyer = false;
                    destroyer = null;
                }
            }
        }

        public void MouseMove(Vector2 pos)
        {
            if (m_mouseJoint != null)
            {
                m_mouseJoint.SetTarget(pos);
            }
        }

    }
}
