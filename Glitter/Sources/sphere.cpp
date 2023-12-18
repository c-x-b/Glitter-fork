#include "sphere.h"

Sphere::Sphere(float _radius, int _slices, int _stacks)
    : radius(_radius), slices(_slices), stacks(_stacks) {

}

Sphere::~Sphere() {}

void Sphere::addTriangle(glm::fvec3 v1, glm::fvec3 v2, glm::fvec3 v3) {
    //adds the three vertices to the points vector if they don't exist there already
	if(find(points.begin(), points.end(), v1) == points.end())
		points.push_back(v1);

	if(find(points.begin(), points.end(), v2) == points.end())
		points.push_back(v2);

	if(find(points.begin(), points.end(), v3) == points.end())
		points.push_back(v3);

	//calculate the index of the vertices in the points vector
	unsigned int index1 = find(points.begin(), points.end(), v1) - points.begin();
	unsigned int index2 = find(points.begin(), points.end(), v2) - points.begin();
	unsigned int index3 = find(points.begin(), points.end(), v3) - points.begin();

	//calculate the face normal
	glm::fvec3 n1 = v2 - v1;
	glm::fvec3 n2 = v3 - v1;
	glm::fvec3 n = cross(n1, n2);

	auto normalIter = find(normals.begin(), normals.end(), n);
	//insert normal in the normals vector if not there already
	if (normalIter == normals.end()){
		normals.push_back(n);
		// because we want the index of the last added normal duh
		normalIter = normals.end() - 1;
	}


	//calculate the normal index
	unsigned int normalIndex = normalIter - normals.begin();

	//make a face with three vertex indices and a normal index
	face* f = new face(index1, index2, index3, normalIndex);
	faces.push_back(f);

	std::vector<glm::fvec3> triangleVertices;
	triangleVertices.push_back(v1);
	triangleVertices.push_back(v2);
	triangleVertices.push_back(v3);

	//insert the normal index value for all the three vertex keys in the map
	for(unsigned i = 0; i < triangleVertices.size(); i++){
		glm::fvec3 vertex = triangleVertices.at(i);
		if(sharedNormals.find(vertex) == sharedNormals.end()){
			std::vector<glm::fvec3> temp;
			temp.push_back(n);
			sharedNormals.insert(std::pair<glm::fvec3, std::vector<glm::fvec3>>(vertex, temp));
		}else{
			auto& normalList = sharedNormals.at(vertex);
			normalList.push_back(n);
		}
	}
}

void Sphere::addTriangle(glm::fvec3 v1, glm::fvec3 u1, glm::fvec3 v2, glm::fvec3 u2, glm::fvec3 v3, glm::fvec3 u3) {
    //add triangle normally
	addTriangle(v1, v2, v3);

	//TODO integrate uv coordinates in face
	//add extra texture coordinates to the uv vector
	uvs.push_back(u1);
	uvs.push_back(u2);
	uvs.push_back(u3);

	glm::fvec3 e1 = v2 - v1;
	glm::fvec3 e2 = v3 - v1;

	float deltaU1 = u2.x - u1.x;
	float deltaV1 = u2.y - u1.y;
	float deltaU2 = u3.x - u1.x;
	float deltaV2 = u3.y - u1.y;

	float f = 1.f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

	glm::fvec3 tangent;
	tangent = f * (deltaV2 * e1 - deltaV1 * e2);

	auto tangentIter = find(tangents.begin(), tangents.end(), tangent);

	// if tangent not in the vector, then add it and set iter to the
	// vector's tail. 
	if (tangentIter == tangents.end()){
		tangents.push_back(tangent);
		tangentIter = tangents.end() - 1;
	}

	unsigned int tangentIndex = tangentIter - tangents.begin();

	face* face = faces.back();
	face->t = tangentIndex;

	std::vector<glm::fvec3> triangleVertices;
	triangleVertices.push_back(v1);
	triangleVertices.push_back(v2);
	triangleVertices.push_back(v3);
	
	for (unsigned i = 0; i < triangleVertices.size(); i++){
		glm::fvec3 v = triangleVertices.at(i);
		if (sharedTangents.find(v) == sharedTangents.end()){
			sharedTangents.insert(std::pair<glm::fvec3, glm::fvec3>(v, tangent));
		}
		else{
			glm::fvec3& vt = sharedTangents.at(v);
			vt += tangent;
		}
	}	
}

void Sphere::generateMesh() {
    float deltaPhi = (float)PI/stacks;
	float deltaTheta = (2 * (float)PI)/slices;

	for(float phi = 0; phi < PI ; phi += deltaPhi) {
		for(float theta = 0; theta < 2*PI - 0.001; theta += deltaTheta) {

			float x1 = -sinf(phi) * sinf(theta) * radius;
			float y1 = -cosf(phi) * radius;
			float z1 = -sinf(phi) * cosf(theta) * radius;

			float u1 = float( atan2(x1, z1) / (2 * PI) + 0.5 );
			float v1 = float( -asin(y1 / radius) / (float)PI + 0.5 );

			float x2 = -sinf(theta + deltaTheta) * sinf(phi) * radius;
			float y2 = -cosf(phi) * radius;
			float z2 = -sinf(phi) * cosf(theta + deltaTheta) * radius;

			float u2 = float( atan2(x2, z2) / (2 * PI) + 0.5 );
			float v2 = float( -asin(y2 / radius) / ((float)PI) + 0.5 );

			float x3 = -sinf(theta + deltaTheta) * sinf(phi + deltaPhi) * radius;
			float y3 = -cosf(phi + deltaPhi) * radius;
			float z3 = -sinf(phi + deltaPhi) * cosf(theta + deltaTheta) * radius;

			float u3 = float( atan2(x3, z3) / (2 * (float)PI) + 0.5 );
			float v3 = float( -asin(y3 / radius) / (float)PI + 0.5 );

			float x4 = -sinf(theta) * sinf(phi + deltaPhi) * radius;
			float y4 = -cosf(phi + deltaPhi) * radius;
			float z4 = -sinf(phi + deltaPhi) * cosf(theta) * radius;

			float u4 = float( atan2(x4, z4) / (2 * (float)PI) + 0.5 );
			float v4 = float( -asin(y4 / radius) / (float)PI + 0.5 );
		

			glm::fvec3 p1(x1, y1, z1);
			glm::fvec3 uv1(u1, v1, 0);
			glm::fvec3 p2(x2, y2, z2);
			glm::fvec3 uv2(u2, v2, 0);
			glm::fvec3 p3(x3, y3, z3);
			glm::fvec3 uv3(u3, v3, 0);
			glm::fvec3 p4(x4, y4, z4);
			glm::fvec3 uv4(u4, v4, 0);

			//addTriangle(x1, y1, z1, u1, v1,
			//	x2, y2, z2, u2, v2,
			//	x3, y3, z3, u3, v3);

			//addTriangle(x1, y1, z1, u1, v1,
			//	x3, y3, z3, u3, v3,
			//	x4, y4, z4, u4, v4); 

			addTriangle(p1, uv1, p2, uv2, p3, uv3);
			addTriangle(p1, uv1, p3, uv3, p4, uv4);
		}
	}
}

void Sphere::setPosition(const glm::fvec3 &_position) {
    position = _position;
}

// void Sphere::setScale(const glm::fvec3 &_scale) {
//     scale = _scale;
// }

void Sphere::setDiffuse(const glm::fvec3 &_diffuse) {
    diffuse = _diffuse;
}

void Sphere::setTextures(const std::vector<std::pair<std::string, std::string>> &_textures) {
	textures = _textures;
}

void Sphere::initBuffer(Shader &shader) {
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
    glGenBuffers(1, &NormalVBO);
	glGenBuffers(1, &textureVBO);

	glBindBuffer(GL_ARRAY_BUFFER, PosVBO);
    glBufferData(GL_ARRAY_BUFFER, outArraySize * sizeof(float), outVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NormalVBO);
    glBufferData(GL_ARRAY_BUFFER, outArraySize * sizeof(float), outNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);

	if(uvs.size() > 0){
		float *outUV = new float[uvs.size() * 2];
		index = 0;

		for(unsigned i = 0; i < uvs.size(); i++){
			outUV[index] = uvs.at(i).x;
			outUV[index + 1] = uvs.at(i).y;
			index += 2;
		}
		//pass the uv coord buffers
		glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * 2 * sizeof(float), outUV, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		delete[] outUV;
	}

	delete[] outVertices;
	delete[] outNormals;
    delete[] outTangents;
}

void Sphere::render(Shader &shader, TextureManager &textureManager) {
    shader.use();

	for (auto it = textures.begin(); it != textures.end(); it++) {
		textureManager.BindTexture2D(it->first, it->second, shader);
	}

	glm::fmat4 scaleMatrix = glm::scale(glm::fmat4(1.0f), glm::fvec3(1.0f));
	glm::fmat4 translateMatrix = glm::translate(glm::fmat4(1.0f), position);
	glm::fmat4 rotationMatrix_X = glm::rotate(glm::fmat4(1.0f), angle[0], glm::fvec3(1.0f, 0.0f, 0.0f));
	glm::fmat4 rotationMatrix_Y = glm::rotate(glm::fmat4(1.0f), angle[1], glm::fvec3(0.0f, 1.0f, 0.0f));
	glm::fmat4 rotationMatrix_Z = glm::rotate(glm::fmat4(1.0f), angle[2], glm::fvec3(0.0f, 0.0f, 1.0f));

	glm::fmat4 modelMatrix = scaleMatrix * rotationMatrix_X * rotationMatrix_Y * rotationMatrix_Z * translateMatrix;
    shader.setMat4("model", modelMatrix);

    glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, faces.size() * 3 * 3);

	textureManager.unbindAllTextures();
}