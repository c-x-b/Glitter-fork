#include "textureManager.h"

std::unique_ptr<TextureManager> TextureManager::m_Instance = nullptr;
std::once_flag TextureManager::m_onceFlag;

TextureManager::TextureManager()
{
	m_activeTextureCount = 0;
}

TextureManager& TextureManager::GetInstance()
{
	call_once(m_onceFlag,
		[] 
	{
		m_Instance.reset(new TextureManager);
	});

	return *m_Instance.get();
}

void TextureManager::GenerateFBOTexture2D(std::string texAlias, int width, int height, bool isDepth)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, (isDepth ? GL_DEPTH_COMPONENT : GL_RGBA),
		width, height, 0, (isDepth ? GL_DEPTH_COMPONENT : GL_RGBA), GL_FLOAT, NULL);

	//texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (glGetError()){
		std::cout << "Error while creating Empty Texture: " << glGetString(glGetError()) << std::endl;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	m_texIDMap.insert(make_pair(texAlias, textureID));
}

void TextureManager::GenerateTextureRecFromFloats(std::string texAlias, int width, int height, float *data) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_RECTANGLE, textureID);

	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);

	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
	//glDisable(GL_TEXTURE_2D);

	m_texIDMap.insert(make_pair(texAlias, textureID));

	std::cout << "Successfully generate texture, textureID: " << textureID << std::endl;
}

void TextureManager::LoadTexture1D(std::string filename, std::string textureAlias)
{
    std::string actualPath = "../../Glitter/Resources/"+ std::string(filename);

	int width, height, nrComponents;
	unsigned int textureID;

	//glEnable(GL_TEXTURE_1D);

	unsigned char* imgData = NULL;
	imgData = stbi_load(actualPath.c_str(), &width, &height, &nrComponents, 0);

	if (imgData == NULL)
	{
		std::cout << "Error loading Image: " << actualPath << std::endl;
		exit(1);
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_1D, textureID);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8, width, 0, GL_RGB, GL_UNSIGNED_BYTE, imgData);

	stbi_image_free(imgData);
	
	glBindTexture(GL_TEXTURE_1D, 0);
	//glDisable(GL_TEXTURE_1D);

	m_texIDMap.insert(make_pair(textureAlias, textureID));
}

void TextureManager::LoadTexture2D(std::string filename, std::string textureAlias)
{
    std::string actualPath = "../../Glitter/Resources/"+ std::string(filename);
	std::cout << "Start load texture: " << actualPath << std::endl;

	int width, height, nrComponents;
	unsigned int textureID;

	//glEnable(GL_TEXTURE_2D);

	unsigned char* imgData = NULL;
	imgData = stbi_load(actualPath.c_str(), &width, &height, &nrComponents, 0);

	GLenum format;
	if (nrComponents == 1)
		format = GL_RED;
	else if (nrComponents == 3)
		format = GL_RGB;
	else if (nrComponents == 4)
		format = GL_RGBA;

	std::cout << "stbi_load finished " << std::endl;
	
	if (imgData == NULL)
	{
		std::cout << "Error loading Image: " << actualPath << std::endl;
		exit(1);
	}
	
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imgData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// if (GLEW_EXT_texture_filter_anisotropic)
	// {
	// 	GLfloat maxAnisotropySamples;
	// 	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropySamples);
	// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropySamples);

	// }

	stbi_image_free(imgData);

	glBindTexture(GL_TEXTURE_2D, 0);
	//glDisable(GL_TEXTURE_2D);

	m_texIDMap.insert(make_pair(textureAlias, textureID));

	std::cout << "Successfully load texture, textureID: " << textureID << std::endl;
}

void TextureManager::LoadTextureCubeMap(std::vector<std::string> textureFaces, std::string textureAlias)
{
	int width, height, nrComponents;
	unsigned int textureID;
	unsigned char* imgData = NULL;

	//glEnable(GL_TEXTURE_CUBE_MAP);
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (unsigned int i = 0; i < textureFaces.size(); i++)
	{
        std::string actualPath = "../../Glitter/Resources/"+ std::string(textureFaces[i]);
		imgData = stbi_load(actualPath.c_str(), &width, &height, &nrComponents, 0);
		std::cout << width << " " << height << std::endl;
		if (imgData == NULL)
		{
			std::cout << "Error loading Image!" << std::endl;
			exit(1);
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imgData);
		stbi_image_free(imgData);
		imgData = NULL;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	//glDisable(GL_TEXTURE_CUBE_MAP);

	m_texIDMap.insert(make_pair(textureAlias, textureID));
}

void TextureManager::BindTexture1D(std::string texAlias, std::string sampler, unsigned int program)
{
	assert(m_texIDMap.count(texAlias) != 0);
	glUseProgram(program);
	//glEnable(GL_TEXTURE_1D);
	unsigned int currentTextureID = m_texIDMap.at(texAlias);
	glActiveTexture(GL_TEXTURE0 + m_activeTextureCount);
	glBindTexture(GL_TEXTURE_1D, currentTextureID);
	glUniform1i(glGetUniformLocation(program, sampler.c_str()), m_activeTextureCount);
	TextureUnit currTextureUnit = { GL_TEXTURE0 + m_activeTextureCount, GL_TEXTURE_1D, currentTextureID };
	m_activeTextureUnits.push_back(currTextureUnit);
	m_activeTextureCount++;
}

void TextureManager::BindTexture2D(std::string texAlias, std::string sampler, Shader &shader)
{
	//std::cout << "BindTexture2D " << texAlias << " " << sampler << std::endl;
	assert(m_texIDMap.count(texAlias) != 0);
	shader.use();
	//glEnable(GL_TEXTURE_2D);
	//std::cout << glGetError() << std::endl;
	unsigned int currentTextureID = m_texIDMap.at(texAlias);
	glActiveTexture(GL_TEXTURE0 + m_activeTextureCount);
	//std::cout << glGetError() << std::endl;
	glBindTexture(GL_TEXTURE_2D, currentTextureID);
	//std::cout << glGetError() << std::endl;
	shader.setInt(sampler.c_str(), m_activeTextureCount);
	//std::cout << "bind end " << glGetError() << std::endl;
	TextureUnit currTextureUnit = { GL_TEXTURE0 + m_activeTextureCount, GL_TEXTURE_2D, currentTextureID };
	m_activeTextureUnits.push_back(currTextureUnit);
	m_activeTextureCount++;
}

void TextureManager::BindTextureRec(std::string texAlias, std::string sampler, Shader &shader)
{
	//std::cout << "BindTexture2D " << texAlias << " " << sampler << std::endl;
	assert(m_texIDMap.count(texAlias) != 0);
	shader.use();
	//glEnable(GL_TEXTURE_2D);
	//std::cout << glGetError() << std::endl;
	unsigned int currentTextureID = m_texIDMap.at(texAlias);
	glActiveTexture(GL_TEXTURE0 + m_activeTextureCount);
	//std::cout << glGetError() << std::endl;
	glBindTexture(GL_TEXTURE_RECTANGLE, currentTextureID);
	//std::cout << glGetError() << std::endl;
	shader.setInt(sampler.c_str(), m_activeTextureCount);
	//std::cout << "bind end " << glGetError() << std::endl;
	TextureUnit currTextureUnit = { GL_TEXTURE0 + m_activeTextureCount, GL_TEXTURE_RECTANGLE, currentTextureID };
	m_activeTextureUnits.push_back(currTextureUnit);
	m_activeTextureCount++;
}

void TextureManager::BindTextureCubeMap(std::string texAlias, std::string sampler, unsigned int program)
{
	assert(m_texIDMap.count(texAlias) != 0);
	glUseProgram(program);
	//glEnable(GL_TEXTURE_CUBE_MAP);
	unsigned int currentTextureID = m_texIDMap.at(texAlias);
	glActiveTexture(GL_TEXTURE0 + m_activeTextureCount);
	glBindTexture(GL_TEXTURE_CUBE_MAP, currentTextureID);
	glUniform1i(glGetUniformLocation(program, sampler.c_str()), m_activeTextureCount);
	TextureUnit currTextureUnit = { GL_TEXTURE0 + m_activeTextureCount, GL_TEXTURE_CUBE_MAP, currentTextureID };
	m_activeTextureUnits.push_back(currTextureUnit);
	m_activeTextureCount++;
}

void TextureManager::unbindTexture(std::string texAlias)
{
	assert(m_texIDMap.count(texAlias) != 0);
	unsigned int texID = m_texIDMap.at(texAlias);
	auto currentTextureUnit = find_if(m_activeTextureUnits.begin(), m_activeTextureUnits.end(), [texID](const TextureUnit& t){
		return t.m_ID == texID;
	});

	glActiveTexture(currentTextureUnit->m_activeUnit);
	glBindTexture(currentTextureUnit->m_target, 0);
	//glDisable(currentTextureUnit->m_target);
	m_activeTextureUnits.erase(currentTextureUnit);
	m_activeTextureCount--;
}

void TextureManager::unbindAllTextures()
{
	for (auto currentTextureUnit = m_activeTextureUnits.rbegin(); currentTextureUnit != m_activeTextureUnits.rend(); currentTextureUnit++)
	{
		glActiveTexture(currentTextureUnit->m_activeUnit);
		glBindTexture(currentTextureUnit->m_target, 0);
		//glDisable(currentTextureUnit->m_target);
	}
	m_activeTextureUnits.clear();
	m_activeTextureCount = 0;
}