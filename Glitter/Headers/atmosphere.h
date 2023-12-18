#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H
#pragma once

#include "sphere.h"

class atmosphereSphere : public Sphere {
    // 总计算量: (size * size * 3) * samples
    const int tableSize = 500;
    const int integralSamples = 100;
    float* lookUpTable; //[距地面高度][与竖直向上的角度][3(RGB)]   exp{-β(λ)D(P)}
    float earthRadius, atmosphereThickness;
    glm::fvec3 rayleighTerm;

    static float scatteringCoefficient(float h, float _atmosphereThickness); //总散射系数，积分后就是衰减系数，h为海拔
    glm::fvec3 solveHit(glm::fvec3 start, glm::fvec3 dir);
    float calcRayleighSea(float waveLength);
    float calcIntegral(glm::fvec3 start, glm::fvec3 end, float baseRadius, int samples, float (* calcFunc)(float, float));

public:
    atmosphereSphere(float _radius, int _slices, int _stacks);
    ~atmosphereSphere();

    void setEarthRadius(float _earthRadius);
    void calcLookUpTable();
    void generateLUTTexture(TextureManager &textureManager);
    void initBuffer(Shader &shader);
    void render(Shader &shader, TextureManager &textureManager);
};

#endif // !ATMOSPHERE_H
