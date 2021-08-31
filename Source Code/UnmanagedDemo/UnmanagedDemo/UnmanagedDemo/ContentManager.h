#pragma once
#ifndef  _CONTENTMANAGER_H_
#define _CONTENTMANAGER_H_

#include <string>
#include "Texture2D.h"

//Based on the syntax for the XNA content manager. Templated for future extension
class ContentManager
{
public:
	ContentManager(void);
	~ContentManager(void);

	template<typename T>
	T* Load(std::string resourceName);
};

template<> Texture2D* ContentManager::Load<Texture2D>(std::string resourceName);

template<typename T>
T* ContentManager::Load(std::string resourceName)
{
	return NULL;
}

#endif // ! _CONTENTMANAGER_H_