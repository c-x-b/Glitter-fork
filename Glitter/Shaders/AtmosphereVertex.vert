#version 330 core
precision highp float;
layout (location = 0) in vec3 aPos;

// set in main
uniform vec3 cameraPos;
uniform float cameraHeight;
uniform float cameraHeight2;
uniform vec3 sunLightDir;
uniform float atmosphereRadius;
uniform float atmosphereRadius2;
uniform float earthRadius;
uniform float const3Divide16PI; // 3/(16PI)
uniform float PI;
uniform mat4 view;
uniform mat4 projection;

// set in render
uniform sampler2DRect RLUT;
uniform sampler2DRect MLUT;
uniform mat4 model;
uniform vec3 rayleighTerm; // β(λ)
uniform vec3 mieTerm;
uniform int samples;
uniform int LUTTableSize;
uniform float rayleighBaseRate;
uniform float mieBaseRate;
uniform float g;
uniform float g2; // g*g

out vec3 rayleighResult;
out vec3 mieResult;

vec3 findFromLUT(sampler2DRect LUT, float height, float cosValue) {
    float angle = acos(cosValue);
    float normalizedAngle = angle / PI;
    //vec4 findResult = texelFetch(LUT, ivec2(normalizedAngle * LUTTableSize, height * LUTTableSize), 0);
    vec4 findResult = texture(LUT, vec2(normalizedAngle * LUTTableSize, height * LUTTableSize));
    return findResult.rgb;
}

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    vec3 outPos = vec3(model * vec4(aPos, 1.0));
    vec3 sightRay = outPos - cameraPos;
    float rayLength = length(sightRay); // length of the light ray in atmosphere
    sightRay /= rayLength; // normalized

    vec3 startPos;
    bool outAtmosphere;

    // float B = 2.0 * dot(cameraPos, sightRay);
	// float C = cameraHeight2 - atmosphereRadius2;
	// float fDet = max(0.0, B * B - 4.0 * C);
	// float fNear = 0.5 * (-B - sqrt(fDet));

    float tmp = dot(-cameraPos, sightRay);
    float halfChord = rayLength - tmp;
    float rayHeight = sqrt(cameraHeight2 - tmp * tmp);
    if (cameraHeight > atmosphereRadius) {
        startPos = cameraPos + (rayLength - 2 * halfChord) * sightRay;
        rayLength = 2 * halfChord;
        outAtmosphere = true;
    }
    else {
        startPos = cameraPos;
        outAtmosphere = false;
    }

    vec3 normalizedSunLight = normalize(sunLightDir);
    float atmosphereThickness = atmosphereRadius - earthRadius;
    float cosSunAndSight = dot(normalizedSunLight, sightRay);
    float rayleighPhaseTerm = (1.0 + cosSunAndSight * cosSunAndSight) * const3Divide16PI;
    float miePhaseTerm = (1.0 - g2) * (1.0 + cosSunAndSight * cosSunAndSight) / ((2 + g2) * pow(1 + g2 - 2 * g * cosSunAndSight, 1.5)) * 2 * const3Divide16PI;
    vec3 deltaRay = rayLength / samples * sightRay;
    vec3 samplePoint = startPos + deltaRay * 0.5; 
    vec3 rayleighColor = vec3(0.0);
    vec3 mieColor = vec3(0.0);
    
    // if (outAtmosphere) { // out of atmosphere, entire sight term from LUT
        for (int i = 0; i < samples; i++) {
            float pointHeight = length(samplePoint);
            float heightRate = max(0.0, (pointHeight - earthRadius) / atmosphereThickness);
            float cosSight = -dot(sightRay, samplePoint) / pointHeight;
            float cosSunlight = dot(normalizedSunLight, samplePoint) / pointHeight;
            vec3 RLUTTerm = findFromLUT(RLUT, heightRate, cosSight) * findFromLUT(RLUT, heightRate, cosSunlight);
            rayleighColor += RLUTTerm * exp(-heightRate / rayleighBaseRate);

            vec3 MLUTTerm = findFromLUT(MLUT, heightRate, cosSight) * findFromLUT(MLUT, heightRate, cosSunlight);
            mieColor += MLUTTerm * exp(-heightRate / mieBaseRate);

            samplePoint += deltaRay;
        }
    // }
    // else
    // if (rayHeight < earthRadius - 100.0 && rayLength > halfChord) { // ray intersect with earth, will be covered
    //     rayleighColor = vec3(0.0, 0.0, 0.0);
    // }
    // else { // could calc sight term by Camera-Atmosphere divide Point-Atmosphere
    //     float cosCamera = dot(cameraPos, sightRay) / cameraHeight;
    //     vec2 camera_AtmosphereTerm = findFromLUT((cameraHeight - earthRadius) / atmosphereThickness, cosCamera).rg;
    //     for (int i = 0; i < samples; i++) {
    //         float pointHeight = length(samplePoint);
    //         float heightRate = (pointHeight - earthRadius) / atmosphereThickness;
    //         float cosSunlight = dot(normalizedSunLight, samplePoint) / pointHeight;
    //         vec2 lightTerm = findFromLUT(heightRate, cosSunlight).rg;
    //         float cosSight = dot(sightRay, samplePoint) / pointHeight;
    //         vec2 LUTTerm = lightTerm + camera_AtmosphereTerm - findFromLUT(heightRate, cosSight).rg;
    //         vec3 RwithinExp = rayleighTerm * LUTTerm.r; // rayleigh
    //         vec3 Rtmp = vec3(exp(-RwithinExp.x), exp(-RwithinExp.y), exp(-RwithinExp.z));
    //         rayleighColor += Rtmp * exp(-heightRate / rayleighBaseRate);
    //         vec3 MwithinExp = mieTerm * LUTTerm.g * 1.1; // mie
    //         vec3 Mtmp = vec3(exp(-MwithinExp.x), exp(-MwithinExp.y), exp(-MwithinExp.z));
    //         mieColor += Mtmp * exp(-heightRate / mieBaseRate);
    //         samplePoint += deltaRay;
    //     }
    // }
    //rayleighResult = rayleighColor;
    rayleighResult = rayleighColor * (rayLength / samples) * rayleighPhaseTerm * rayleighTerm;
    mieResult = mieColor * (rayLength / samples) * miePhaseTerm * mieTerm;
}