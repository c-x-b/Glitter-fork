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
    const float rayleighBaseRate = 0.2f; // 瑞利用exp(-h/H)计算光学距离时, H /大气层厚度的值
    glm::fvec3 mieTerm;
    const float mieBaseRate = 0.05f; //与上同理
    const float g = 0.75f; // 影响米氏散射的相位函数

    glm::fvec3 solveHit(glm::fvec3 start, glm::fvec3 dir);
    float calcRayleighTerm(float waveLength);
    glm::fvec2 calcOpticalDepth(glm::fvec3 start, glm::fvec3 end, float baseRadius, int samples);

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
