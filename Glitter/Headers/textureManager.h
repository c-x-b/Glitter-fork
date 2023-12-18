#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H
#pragma once

#include "shader.h"

#include <stb_image.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <cassert>

struct TextureUnit
{
	unsigned int m_activeUnit;
	unsigned int m_target;
	unsigned int m_ID;
};

class TextureManager{

	TextureManager();
	TextureManager(const TextureManager&) = delete;
	const TextureManager& operator=(const TextureManager&) = delete;

	static std::unique_ptr<TextureManager> m_Instance;
	static std::once_flag m_onceFlag;

	std::map<std::string, unsigned int> m_texIDMap;
	//map<unsigned int, unsigned int> m_textureTargets;
    std::vector<TextureUnit> m_activeTextureUnits;
    unsigned int m_activeTextureCount; 

public:

	static TextureManager& GetInstance();
	void LoadTexture2D(std::string filename, std::string textureAlias);
	void LoadTexture1D(std::string filename, std::string textureAlias);
	void LoadTextureCubeMap(std::vector<std::string> textureFaces, std::string textureAlias);
	void GenerateFBOTexture2D(std::string texAlias, int width, int height, bool isDepth = false);
	void GenerateTexture2DFromFloats(std::string texAlias, int width, int height, float *data);
	void BindTexture2D(std::string texAlias, std::string sampler, Shader &shader);
	void BindTexture1D(std::string texAlias, std::string sampler, unsigned int program);
	void BindTextureCubeMap(std::string texAlias, std::string sampler, unsigned int program);
	void unbindTexture(std::string texAlias);
	void unbindAllTextures();

	const unsigned int& operator[] (const std::string& texAlias)
	{
		assert(m_texIDMap.count(texAlias) != 0);
		if (m_texIDMap.find(texAlias) == m_texIDMap.end())
			return 0;
		else
			return m_texIDMap.at(texAlias);
	}
};

#endif
