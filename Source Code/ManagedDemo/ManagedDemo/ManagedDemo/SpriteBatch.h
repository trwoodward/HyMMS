#pragma once
#include <gl/GL.h>
#include <gl/GLU.h>

#include "DemoRectangle.h"
#include "DemoColour.h"
#include <vector>
#include "Texture2D.h"
#include <math.h>
#include "HyMMS.h"

//Based on the class of the same name from Microsoft's XNA framework
class SpriteBatch : public Managed_Object
{
public:
	SpriteBatch(void);
	~SpriteBatch(void);

	void Begin(); //Clears the sprites array
	void Draw(SmartHandle<Texture2D> texture, DemoRectangle destRect, DemoColour col);
	void Draw(SmartHandle<Texture2D> texture, float* position, DemoRectangle* srcRect, DemoColour col, float angle, float* origin, float* scale);
	void End(); //Constructs an array of vertices, indices, and colours, and draws them using glDrawMultiElements

private:
	struct Sprite
	{
		SmartHandle<Texture2D> texture;
		float vertices[8];
		float texCoords[8];
		DemoColour col;

		Sprite() {}
		Sprite( SmartHandle<Texture2D> a_texture, DemoRectangle a_rect, DemoColour a_col) : texture(a_texture), col(a_col) 
		{
			vertices[0] = (float)a_rect.X();
			vertices[1] = (float)a_rect.Y();
			vertices[2] = (float)a_rect.X();
			vertices[3] = (float)a_rect.Y() + (float)a_rect.Height();
			vertices[4] = (float)a_rect.X() + (float)a_rect.Width();
			vertices[5] = (float)a_rect.Y();
			vertices[6] = (float)a_rect.X() + (float)a_rect.Width();
			vertices[7] = (float)a_rect.Y() + (float)a_rect.Height();

			texCoords[0] = 0.0f;
			texCoords[1] = 0.0f;
			texCoords[2] = 0.0f;
			texCoords[3] = 1.0f;
			texCoords[4] = 1.0f;
			texCoords[5] = 0.0f;
			texCoords[6] = 1.0f;
			texCoords[7] = 1.0f;
		}
		Sprite(SmartHandle<Texture2D> a_texture, float* position, DemoRectangle* a_src_rect, DemoColour a_col, float angle, float* origin, float* scale) : texture(a_texture), col(a_col) 
		{
			//Calculate vertices rotation and displacement
			//Origin is in normalized 2D space. Translate, scale, rotate, translate back by position
			float left = (-(origin[0]) * scale[0]);
			float right = (1 - origin[0]) * scale[0];
			float top = (-(origin[1]) * scale[1]);
			float bottom = (1 - origin[1]) * scale[1];

			float sinA = sin(angle);
			float cosA = cos(angle);

			float topLeftX = (left * cosA) - (top * sinA);
			float topLeftY = (left * sinA) + (top * cosA);
			float bottomLeftX = (left * cosA) - (bottom * sinA);
			float bottomLeftY = (left * sinA) + (bottom * cosA);
			float topRightX = (right * cosA) - (top * sinA);
			float topRightY = (right * sinA) + (top * cosA);
			float bottomRightX = (right * cosA) - (bottom * sinA);
			float bottomRightY = (right * sinA) + (bottom * cosA);

			vertices[0] = topLeftX + position[0];
			vertices[1] = topLeftY + position[1];
			vertices[2] = bottomLeftX + position[0];
			vertices[3] = bottomLeftY + position[1];
			vertices[4] = topRightX + position[0];
			vertices[5] = topRightY + position[1];
			vertices[6] = bottomRightX + position[0];
			vertices[7] = bottomRightY + position[1];
			
			if (a_src_rect)
			{
				float x = (float)a_src_rect->X() / (float)texture->GetWidth();
				float y = (float)a_src_rect->Y() / (float)texture->GetHeight();
				float width = (float)a_src_rect->Width() / (float)texture->GetWidth();
				float height = (float)a_src_rect->Height() / (float)texture->GetHeight();
				texCoords[0] = x;
				texCoords[1] = y;
				texCoords[2] = x;
				texCoords[3] = y + height;
				texCoords[4] = x + width;
				texCoords[5] = y;
				texCoords[6] = x + width;
				texCoords[7] = y + height;
			}
			else
			{
				texCoords[0] = 0.0f;
				texCoords[1] = 0.0f;
				texCoords[2] = 0.0f;
				texCoords[3] = 1.0f;
				texCoords[4] = 1.0f;
				texCoords[5] = 0.0f;
				texCoords[6] = 1.0f;
				texCoords[7] = 1.0f;
			}
		}
	};
	
	std::vector<Sprite> sprites;
};

