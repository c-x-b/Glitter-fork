#ifndef ATMOSPHERESTATE_H
#define ATMOSPHERESTATE_H

#include "shader.h"
#include "textureManager.h"
#include <glm/glm.hpp>
#include <cassert>
#include <iostream>

#define PI 3.14159

namespace AtmosphereState {
const float earthRadius = 6360e3f;
const float atmosphereThickness = earthRadius * 0.015f;
const float atmosphereRadius = earthRadius + atmosphereThickness;

const float const3Divide16PI = 3.0f / (16.0f * PI);

struct{
    const int tableSize = 512;
    const int integralSamples = 10;
    float *LookUpTable; //[距地面高度][与竖直向上的角度][3(RGB)]   exp{-β(λ)D(P)}
    glm::fvec3 rayleighTerm;
    const float rayleighBaseRate = 0.25f; // 瑞利用exp(-h/H)计算光学距离时, H /大气层厚度的值
    glm::fvec3 mieTerm;
    const float mieBaseRate = 0.02f; //与上同理
    const float g = -0.99f; // 影响米氏散射的相位函数

    bool usable = false;

    std::vector<std::pair<std::string, std::string>> textures; //pair(name in manager, sampler name in shader)

    glm::fvec2 solveHit(glm::fvec3 start, glm::fvec3 dir, float r) {
        // len(start + dir * x) = atmosphereRadius, 当一元二次方程来解
        // a = dir.x^2 + dir.y^2 + dir.z^2 = len(dir)^2
        // b = 2 * start.x * dir.x + 2 * start.y * dir.y + 2 * start.z * dir.z
        // c = len(start)^2 - atmosphereRadius^2
        float a = dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2];
        float b = 2 * (dir[0] * start[0] + dir[1] * start[1] + dir[2] * start[2]);
        float c = start[0] * start[0] + start[1] * start[1] + start[2] * start[2] - r * r;
        float delta = b * b - 4 * a * c;
        assert(delta >= 0);
        float tmp1 = -b / (2 * a);
        float tmp2 = sqrtf(delta) / (2 * a);
        float x1 = tmp1 - tmp2, x2 = tmp1 + tmp2;
        if (x1 <= 0.0f && x2 > 0.0f)
            return glm::fvec2(x2, x1);
        else if (x1 > 0.0f && x2 > 0.0f)
            return glm::fvec2(x1, x2);
        else {
            // std::cout << "x1: " << x1 << " x2: " << x2 << std::endl;
            // std::cout << start.x << "," << start.y << "," << start.z << "\n"
            //           << dir.x << "," << dir.y << "," << dir.z << std::endl;
            // assert(false);
            return glm::fvec2(x1, x2);
        }
    }

    float calcRayleighTerm(float waveLength) {
        const float n = 1.00029f; //空气折射率
        const float N = 2.504e25f;
        float tmp = waveLength * waveLength * N * waveLength * waveLength;
        float result = 8 * PI * PI * PI / 3.0f;
        result = result * (n * n - 1.0f) * (n * n - 1.0f) / tmp;
        return result;
    }

    glm::fvec2 calcOpticalDepth(glm::fvec3 start, glm::fvec3 end, float baseRadius, int samples) {
        glm::fvec2 result = glm::fvec2(0.0f);
        glm::fvec3 delta = (end - start) / (float)samples;
        for (int i = 0; i < samples;i++) {
            glm::fvec3 pos = start + delta * (float)i;
            float h = glm::length(pos) - baseRadius;
            if (h <= 0.0f) {
                result = glm::fvec2(-1.0f, -1.0f);
                break;
            }
            result[0] += exp(-h / (atmosphereThickness * rayleighBaseRate));
            result[1] += exp(-h / (atmosphereThickness * mieBaseRate));
        }
        result *= glm::length(delta);
        return result;
    }

    void calcLookUpTable() {
        assert(usable);
        
        //rayleighTerm = glm::fvec3(calcRayleighTerm(6.8e-7f), calcRayleighTerm(5.5e-7f), calcRayleighTerm(4.4e-7f));
        rayleighTerm = glm::fvec3(5.8e-6f, 13.5e-6f, 33.1e-6f);
        //rayleighTerm = glm::fvec3(33.1e-6f, 33.1e-6f, 33.1e-6f);
        mieTerm = glm::fvec3(21e-6f);
        std::cout << rayleighTerm.r << " " << rayleighTerm.g << " " << rayleighTerm.b << std::endl;
        float deltaHeight = atmosphereThickness / (float)tableSize;
        float deltaAngle = PI / (float)tableSize;
        int index = 0;
        for (int heightIt = 0; heightIt < tableSize; heightIt++) {
            float height = earthRadius + deltaHeight * (0.5f + (float)heightIt);
            for (int angleIt = 0; angleIt < tableSize; angleIt++)
            {
                //std::cout << "(" << heightIt << "," << angleIt << ") ";
                float angle = deltaAngle * (0.5f + (float)angleIt);
                glm::fvec3 start(0.0f, height, 0.0f);
                glm::fvec3 hitDir(sinf(angle), cosf(angle), 0.0f);
                float rayHeight = height * hitDir[0];
                glm::fvec2 hitLength = solveHit(start, hitDir, atmosphereRadius);
                glm::fvec3 end = start + hitDir * hitLength[0];
                glm::fvec2 opticalDepth = calcOpticalDepth(start, end, earthRadius, integralSamples);

                if (opticalDepth[0] < 0.0f) {
                    LookUpTable[index * 3] = 0.0f;
                    LookUpTable[index * 3 + 1] = 0.0f;
                    LookUpTable[index * 3 + 2] = 0.0f;
                }
                else {
                    LookUpTable[index * 3] = exp(-opticalDepth[0] * rayleighTerm.r - opticalDepth[1] * mieTerm.r * 1.1f );
                    LookUpTable[index * 3 + 1] = exp(-opticalDepth[0] * rayleighTerm.g - opticalDepth[1] * mieTerm.g * 1.1f);
                    LookUpTable[index * 3 + 2] = exp(-opticalDepth[0] * rayleighTerm.b - opticalDepth[1] * mieTerm.b * 1.1f);
                }
                //lookUpTable[index * 3 + 2] = 0.0f;
                // if (heightIt > 490)
                //     std::cout << heightIt << ", " << angleIt << " : (" << lookUpTable[index * 3] << ", " << lookUpTable[index * 3 + 1] << "), ";
                index++;
            }
        }
        std::cout << "calc LUT done" << std::endl;
    }

    void generateLUTTexture(TextureManager &textureManager) {
        textures.push_back(std::make_pair("AtmosphereLUT", "LUT"));
        textureManager.GenerateTextureRecFromFloats("AtmosphereLUT", tableSize, tableSize, LookUpTable);
    }

    void setUniforms(Shader &shader, TextureManager &textureManager) {
        shader.use();

        for (auto it = textures.begin(); it != textures.end(); it++) {
            textureManager.BindTextureRec(it->first, it->second, shader);
        }

        shader.setVec3("rayleighTerm", rayleighTerm);
        shader.setVec3("mieTerm", mieTerm);
        shader.setInt("samples", 50);
        shader.setInt("LUTTableSize", tableSize);
        shader.setFloat("rayleighBaseRate", rayleighBaseRate);
        shader.setFloat("mieBaseRate", mieBaseRate);
        shader.setFloat("g", g);
        shader.setFloat("g2", g * g);
    }

    void init(TextureManager &textureManager) {
        LookUpTable = new float[tableSize * tableSize * 3];
        usable = true;
        calcLookUpTable();
        generateLUTTexture(textureManager);
    }

    void releaseLUT() {
        delete[] LookUpTable;
        usable = false;
    }

} LUT; 

}; // namespace AtmosphereState
#endif