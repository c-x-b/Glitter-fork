#ifndef SPHERE_H
#define SPHERE_H
#pragma once

#include "face.h"
#include "shader.h"
#include "textureManager.h"
#include "glm\gtc\matrix_transform.hpp"
#include <vector>
#include <map>
#include <algorithm>

class Sphere {

    std::vector<glm::fvec3> points;
    std::vector<glm::fvec3> tangents;
    std::vector<glm::fvec3> normals;
    std::vector<glm::fvec3> uvs;
    std::vector<face*> faces;

    //a map<vertex, vector<normals>> to store the normals which are shared by each vertex
    std::map<glm::fvec3, std::vector<glm::fvec3>, fvec3Comp> sharedNormals;
	std::map<glm::fvec3, glm::fvec3, fvec3Comp> sharedTangents;

    float radius;
	int slices, stacks;
    glm::fvec3 position; 
    glm::fvec3 scale;
    glm::fvec3 angle;
    glm::fvec3 diffuse;

    unsigned int VAO;
    unsigned int PosVBO, NormalVBO, textureVBO;

    std::vector<std::pair<std::string, std::string>> textures; //pair(name in manager, sampler name in shader)

public:
    Sphere(float _radius, int _slices, int _stacks);
    ~Sphere();

    void addTriangle(glm::fvec3 v1, glm::fvec3 v2, glm::fvec3 v3); //without texture
    void addTriangle(glm::fvec3 v1, glm::fvec3 u1, glm::fvec3 v2, glm::fvec3 u2, glm::fvec3 v3, glm::fvec3 u3);
    void generateMesh();
    void setPosition(const glm::fvec3 &_position);
    void setScale(const glm::fvec3 &_scale);
    void setDiffuse(const glm::fvec3 &_diffuse);
    void setTextures(const std::vector<std::pair<std::string, std::string>> &_textures);

    void initBuffer(Shader &shader);
    void render(Shader &shader, TextureManager &textureManager);
};

#endif