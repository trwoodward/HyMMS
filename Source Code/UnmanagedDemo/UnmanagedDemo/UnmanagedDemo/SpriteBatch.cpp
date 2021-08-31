#include "stdafx.h"
#include "SpriteBatch.h"


SpriteBatch::SpriteBatch(void)
{
	sprites = std::vector<Sprite>();
}


SpriteBatch::~SpriteBatch(void)
{
}

void SpriteBatch::Begin()
{
	sprites.clear();
}

void SpriteBatch::Draw( Texture2D* texture, DemoRectangle destRect, DemoColour col )
{
	//Currently draws them in the order that they are stated
	//Use single texture at the moment. Since each array is drawn 
	//separately can't batch them together, so just switch texture for each sprite.
	sprites.push_back(Sprite(texture, destRect, col));
}

void SpriteBatch::Draw( Texture2D* texture, float* position, DemoRectangle* srcRect, DemoColour col, float angle, float* origin, float* scale )
{
	sprites.push_back(Sprite(texture, position, srcRect, col, angle, origin, scale));
}

void SpriteBatch::End()
{
	float* verts = new float[3 * 4 * sprites.size()];
	float* cols = new float[4 * 4 * sprites.size()];
	int* indices = new int[sprites.size()];
	int* counts = new int[sprites.size()];
	float* texCoords = new float[2 * 4 * sprites.size()];

	for (int i = 0; i < static_cast<int>(sprites.size()); ++i)
	{
		int indexBase = 12 * i;

		// Each vertex is (x, y)
		verts[indexBase] = sprites[i].vertices[0];
		verts[indexBase + 1] = sprites[i].vertices[1];
		verts[indexBase + 2] = 0.5f;
		verts[indexBase + 3] = sprites[i].vertices[2];
		verts[indexBase + 4] = sprites[i].vertices[3];
		verts[indexBase + 5] = 0.5f;
		verts[indexBase + 6] = sprites[i].vertices[4];
		verts[indexBase + 7] = sprites[i].vertices[5];
		verts[indexBase + 8] = 0.5f;
		verts[indexBase + 9] = sprites[i].vertices[6];
		verts[indexBase + 10] = sprites[i].vertices[7];
		verts[indexBase + 11] = 0.5f;

		float r = sprites[i].col.R();
		float g = sprites[i].col.G();
		float b = sprites[i].col.B();
		float a = sprites[i].col.A();

		int colIndexBase = 16 * i;

		// Each col is (r, g, b, a)
		for (int j = 0; j < 4; ++j)
		{
			cols[colIndexBase + (j * 4)] = r;
			cols[colIndexBase + (j * 4) + 1] = g;
			cols[colIndexBase + (j * 4) + 2] = b;
			cols[colIndexBase + (j * 4) + 3] = a;
		}

		indices[i] = 4 * i;
		counts[i] = 4;

		int texIndexBase = 8 * i;
		texCoords[texIndexBase] = sprites[i].texCoords[0];
		texCoords[texIndexBase + 1] = sprites[i].texCoords[1];
		texCoords[texIndexBase + 2] = sprites[i].texCoords[2];
		texCoords[texIndexBase + 3] = sprites[i].texCoords[3];
		texCoords[texIndexBase + 4] = sprites[i].texCoords[4];
		texCoords[texIndexBase + 5] = sprites[i].texCoords[5];
		texCoords[texIndexBase + 6] = sprites[i].texCoords[6];
		texCoords[texIndexBase + 7] = sprites[i].texCoords[7];
	}

	glVertexPointer(3, GL_FLOAT, 3 * sizeof(float), verts);
	glColorPointer(4, GL_FLOAT, 4 * sizeof(float), cols);
	glTexCoordPointer(2, GL_FLOAT, 2 * sizeof(float), texCoords);

	int currTex = -1;
	for (unsigned int i = 0; i < sprites.size(); ++i)
	{
		if (sprites[i].texture->textureIndex != currTex)
		{
			currTex = sprites[i].texture->textureIndex;
			glBindTexture(GL_TEXTURE_2D, currTex);
		}
		glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
	}
	delete[] verts;
	delete[] cols;
	delete[] indices;
	delete[] counts;
	delete[] texCoords;
}


