#version 330 core
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
uniform sampler2D LUT;
uniform mat4 model;
uniform vec3 rayleighTerm; // β(λ)
uniform int samples;
uniform int LUTTableSize;

out vec3 rayleighResult;

vec3 findFromLUT(float height, float cosValue) {
    float angle = acos(cosValue);
    float normalizedAngle = angle / PI;
    //vec4 findResult = texelFetch(LUT, ivec2(height * LUTTableSize, normalizedAngle * LUTTableSize), 0);
    vec4 findResult = texture(LUT, vec2(normalizedAngle, height));
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
    float halfChord = rayLength - dot(-cameraPos, sightRay);
    float rayHeight = sqrt(atmosphereRadius2 - halfChord * halfChord);
    if (cameraHeight >= atmosphereRadius) {
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
    float PhaseFunctionTerm = (1.0 + cosSunAndSight * cosSunAndSight) * const3Divide16PI;
    vec3 deltaRay = rayLength / samples * sightRay;
    vec3 samplePoint = startPos + deltaRay * 0.5; 
    vec3 resultColor = vec3(0.0);
    if (outAtmosphere) { // out of atmosphere, entire sight term from LUT
        for (int i = 0; i < samples; i++) {
            float pointHeight = length(samplePoint);
            float heightRate = max(0.0, (pointHeight - earthRadius) / atmosphereThickness);
            float cosSight = -dot(sightRay, samplePoint) / pointHeight;
            float cosSunlight = dot(normalizedSunLight, samplePoint) / pointHeight;
            vec3 LUTTerm = findFromLUT(heightRate, cosSight) * findFromLUT(heightRate, cosSunlight);
            resultColor += LUTTerm * exp(-heightRate);
            samplePoint += deltaRay;
        }
    }
    else if (rayHeight < earthRadius && rayLength > halfChord) { // ray intersect with earth, will be covered
        resultColor = vec3(0.0);
    }
    else { // could calc sight term by Camera-Atmosphere divide Point-Atmosphere
        float cosCamera = dot(cameraPos, sightRay) / cameraHeight;
        vec3 camera_AtmosphereTerm = findFromLUT((cameraHeight - earthRadius) / atmosphereThickness, cosCamera);
        for (int i = 0; i < samples; i++) {
            float pointHeight = length(samplePoint);
            float heightRate = (pointHeight - earthRadius) / atmosphereThickness;
            float cosSunlight = dot(normalizedSunLight, samplePoint) / pointHeight;
            vec3 lightTerm = findFromLUT(heightRate, cosSunlight);
            float cosSight = dot(sightRay, samplePoint) / pointHeight;
            vec3 LUTTerm = lightTerm * camera_AtmosphereTerm / findFromLUT(heightRate, cosSight);
            resultColor += LUTTerm * exp(-heightRate);
            samplePoint += deltaRay;
        }
    }
    rayleighResult = resultColor * (rayLength / samples) * PhaseFunctionTerm * rayleighTerm;
}