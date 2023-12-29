#include "atmosphere.h"

atmosphereSphere::atmosphereSphere(float _radius, int _slices, int _stacks)
    : Sphere(_radius, _slices, _stacks) {
    RLookUpTable = new float[tableSize * tableSize * 3];
    MLookUpTable = new float[tableSize * tableSize * 3];
}

atmosphereSphere::~atmosphereSphere() {
    delete[] RLookUpTable;
    delete[] MLookUpTable;
}

void atmosphereSphere::setEarthRadius(float _earthRadius) {
    earthRadius = _earthRadius;
    atmosphereThickness = radius - earthRadius;
}

glm::fvec3 atmosphereSphere::solveHit(glm::fvec3 start, glm::fvec3 dir) {
    // len(start + dir * x) = atmosphereRadius, 当一元二次方程来解
    // a = dir.x^2 + dir.y^2 + dir.z^2 = len(dir)^2
    // b = 2 * start.x * dir.x + 2 * start.y * dir.y + 2 * start.z * dir.z
    // c = len(start)^2 - atmosphereRadius^2
    float a = dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2];
    float b = 2 * (dir[0] * start[0] + dir[1] * start[1] + dir[2] * start[2]);
    float c = start[0] * start[0] + start[1] * start[1] + start[2] * start[2] - radius * radius;
    float delta = b * b - 4 * a * c;
    float tmp1 = -b / (2 * a);
    float tmp2 = sqrtf(delta) / (2 * a);
    float x1 = tmp1 - tmp2, x2 = tmp1 + tmp2;
    if (x1 <= 0.0f && x2 > 0.0f)
        return (start + dir * x2);
    else if (x1 > 0.0f && x2 > 0.0f)
        return (start + dir * x1);
    else {
        std::cout << "x1: " << x1 << " x2: " << x2 << std::endl;
        std::cout << start.x << "," << start.y << "," << start.z << "\n"
                  << dir.x << "," << dir.y << "," << dir.z << std::endl;
        assert(false);
    }
}

float atmosphereSphere::calcRayleighTerm(float waveLength) {
    const float n = 1.00029f; //空气折射率
    const float N = 2.504e25f;
    float tmp = waveLength * waveLength * N * waveLength * waveLength;
    float result = 8 * PI * PI * PI / 3.0f;
    result = result * (n * n - 1.0f) * (n * n - 1.0f) / tmp;
    return result;
}

glm::fvec2 atmosphereSphere::calcOpticalDepth(glm::fvec3 start, glm::fvec3 end, float baseRadius, int samples) {
    glm::fvec2 result = glm::fvec2(0.0f);
    glm::fvec3 delta = (end - start) / (float)samples;
    for (int i = 0; i < samples;i++) {
        glm::fvec3 pos = start + delta * (float)i;
        float h = glm::length(pos) - baseRadius;
        if (h<0.0f) {
            result = glm::fvec2(-1.0f, -1.0f);
            break;
        }
        result[0] += exp(-h / (atmosphereThickness * rayleighBaseRate));
        result[1] += exp(-h / (atmosphereThickness * mieBaseRate));
    }
    result *= glm::length(delta);
    return result;
}

void atmosphereSphere::calcLookUpTable() {
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
            glm::fvec3 end = solveHit(start, hitDir);
            glm::fvec2 opticalDepth = calcOpticalDepth(start, end, earthRadius, integralSamples);

            if (opticalDepth[0] < 0.0f) {
                RLookUpTable[index * 3] = 0.0f;
                RLookUpTable[index * 3 + 1] = 0.0f;
                RLookUpTable[index * 3 + 2] = 0.0f;
                MLookUpTable[index * 3] = 0.0f;
                MLookUpTable[index * 3 + 1] = 0.0f;
                MLookUpTable[index * 3 + 2] = 0.0f;
            }
            else {
                RLookUpTable[index * 3] = exp(-opticalDepth[0] * rayleighTerm.r);
                RLookUpTable[index * 3 + 1] = exp(-opticalDepth[0] * rayleighTerm.g);
                RLookUpTable[index * 3 + 2] = exp(-opticalDepth[0] * rayleighTerm.b);
                MLookUpTable[index * 3] = exp(-opticalDepth[1] * mieTerm.r);
                MLookUpTable[index * 3 + 1] = exp(-opticalDepth[1] * mieTerm.g);
                MLookUpTable[index * 3 + 2] = exp(-opticalDepth[1] * mieTerm.b);
            }
            //lookUpTable[index * 3 + 2] = 0.0f;
            // if (heightIt > 490)
            //     std::cout << heightIt << ", " << angleIt << " : (" << lookUpTable[index * 3] << ", " << lookUpTable[index * 3 + 1] << "), ";
            index++;
        }
    }
    std::cout << "calc LUT done" << std::endl;
}

void atmosphereSphere::generateLUTTexture(TextureManager &textureManager) {
    textures.push_back(std::make_pair("AtmosphereRLUT", "RLUT"));
    textureManager.GenerateTextureRecFromFloats("AtmosphereRLUT", tableSize, tableSize, RLookUpTable);

    textures.push_back(std::make_pair("AtmosphereMLUT", "MLUT"));
    textureManager.GenerateTextureRecFromFloats("AtmosphereMLUT", tableSize, tableSize, MLookUpTable);
}

void atmosphereSphere::initBuffer(Shader &shader) {
    std::cout << "Normals Size: " << normals.size() << std::endl;
    std::cout << "Tangents Size: " << tangents.size() << std::endl;

    int outArraySize = faces.size() * 3 * 3;
    float* outVertices = new float[outArraySize];
	float* outNormals = new float[outArraySize];
    float *outTangents = new float[outArraySize];
    glm::fvec3 v, n, t;
	int index = 0;
	for(unsigned i = 0; i < faces.size(); i++){
		n = normals.at(faces.at(i)->n);
		if (tangents.size() > 0){
			t = tangents.at(faces.at(i)->t);
		}

		for (unsigned j = 0; j < 3; j++){
			v = points.at((*faces.at(i))[j]);
			outVertices[index] = v.x;
			outVertices[index + 1] = v.y;
			outVertices[index + 2] = v.z;

            outNormals[index] = n.x;
            outNormals[index + 1] = n.y;
            outNormals[index + 2] = n.z;

            if (t != glm::fvec3()){
                outTangents[index] = t.x;
                outTangents[index + 1] = t.y;
                outTangents[index + 2] = t.z;
            }

			index += 3;
		}
	}

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &PosVBO);

	glBindBuffer(GL_ARRAY_BUFFER, PosVBO);
    glBufferData(GL_ARRAY_BUFFER, outArraySize * sizeof(float), outVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);

	delete[] outVertices;
	delete[] outNormals;
    delete[] outTangents;
}

void atmosphereSphere::render(Shader &shader, TextureManager &textureManager) {
    shader.use();

    //std::cout << "atmosphere render start " << glGetError() << std::endl;
    for (auto it = textures.begin(); it != textures.end(); it++) {
        textureManager.BindTextureRec(it->first, it->second, shader);
    }
    //std::cout << glGetError() << std::endl;

	glm::fmat4 scaleMatrix = glm::scale(glm::fmat4(1.0f), glm::fvec3(radius, radius, radius));
	glm::fmat4 translateMatrix = glm::translate(glm::fmat4(1.0f), position);
	glm::fmat4 rotationMatrix_X = glm::rotate(glm::fmat4(1.0f), angle[0], glm::fvec3(1.0f, 0.0f, 0.0f));
	glm::fmat4 rotationMatrix_Y = glm::rotate(glm::fmat4(1.0f), angle[1], glm::fvec3(0.0f, 1.0f, 0.0f));
	glm::fmat4 rotationMatrix_Z = glm::rotate(glm::fmat4(1.0f), angle[2], glm::fvec3(0.0f, 0.0f, 1.0f));

	glm::fmat4 modelMatrix = scaleMatrix * rotationMatrix_X * rotationMatrix_Y * rotationMatrix_Z * translateMatrix;
    shader.setMat4("model", modelMatrix);
    shader.setVec3("rayleighTerm", rayleighTerm);
    shader.setVec3("mieTerm", mieTerm);
    shader.setInt("samples", 50);
    shader.setInt("LUTTableSize", tableSize);
    shader.setFloat("rayleighBaseRate", rayleighBaseRate);
    shader.setFloat("mieBaseRate", mieBaseRate);
    shader.setFloat("g", g);
    shader.setFloat("g2", g * g);
    //std::cout << glGetError() << std::endl;

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, faces.size() * 3 * 3);
    // std::cout << "render end " << glGetError() << std::endl;

    textureManager.unbindAllTextures();
}