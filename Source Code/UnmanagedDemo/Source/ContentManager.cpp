#include "stdafx.h"
#include "ContentManager.h"

#include <gl/GL.h>
#include <gl/GLU.h>


ContentManager::ContentManager(void)
{
}


ContentManager::~ContentManager(void)
{
}

//Based on code from Beginning OpenGL Game Programming by Astle and Hawkins, 
//see Project report for full reference
template<>
Texture2D* ContentManager::Load<Texture2D>(std::string resourceName)
{
	//Create new texture, call load on it, register it, and store the index in the textureIndex 
	Texture2D* tex = new Texture2D();

	if (!tex->Load(resourceName.c_str()))
		return NULL;

	glGenTextures(1, &(tex->textureIndex));

	glBindTexture(GL_TEXTURE_2D, tex->textureIndex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, tex->GetWidth(), tex->GetHeight(), GL_RGB, GL_UNSIGNED_BYTE, tex->GetImage());

	return tex;
}
