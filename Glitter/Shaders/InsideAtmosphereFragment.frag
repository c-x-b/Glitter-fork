#version 330 core
precision highp float;
precision highp int;

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
uniform vec3 lightColor;

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

uniform bool mode;

in vec3 actualPos;

out vec4 FragColor;

vec3 findFromLUT(sampler2DRect LUT, float height, float cosValue) {
    float angle = acos(cosValue);
    float normalizedAngle = angle / PI;
    //vec4 findResult = texelFetch(LUT, ivec2(normalizedAngle * LUTTableSize, height * LUTTableSize), 0);
    vec4 findResult = texture(LUT, vec2(normalizedAngle * LUTTableSize, height * LUTTableSize));
    return findResult.rgb;
}

void main() {
    vec3 outPos = normalize(actualPos) * atmosphereRadius;

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
    float rayHeight = sqrt(atmosphereRadius2 - halfChord * halfChord);
    float halfCameraChord = sqrt(atmosphereRadius2 - cameraHeight2);

    startPos = cameraPos;
    outAtmosphere = false;

    vec3 normalizedSunLight = normalize(sunLightDir);
    float atmosphereThickness = atmosphereRadius - earthRadius;
    float cosSunAndSight = dot(normalizedSunLight, sightRay);
    float rayleighPhaseTerm = (1.0 + cosSunAndSight * cosSunAndSight) * const3Divide16PI;
    float miePhaseTerm = (1.0 - g2) * (1.0 + cosSunAndSight * cosSunAndSight) / ((2 + g2) * pow(1 + g2 - 2 * g * cosSunAndSight, 1.5)) * 2 * const3Divide16PI;
    vec3 deltaRay = rayLength / samples * sightRay;
    vec3 samplePoint = startPos + deltaRay * 0.5; 
    vec3 rayleighColor = vec3(0.0);
    vec3 mieColor = vec3(0.0);
    
    if (rayLength <= halfCameraChord) {
        float cosCamera = dot(cameraPos, sightRay) / cameraHeight;
        float cameraHeightRate = max(0.0, (cameraHeight - earthRadius) / atmosphereThickness);
        vec3 cameraRTerm = findFromLUT(RLUT, cameraHeightRate, cosCamera);
        vec3 cameraMTerm = findFromLUT(MLUT, cameraHeightRate, cosCamera);
        for (int i = 0; i < samples; i++) {
            float pointHeight = length(samplePoint);;
            float heightRate = (pointHeight - earthRadius) / atmosphereThickness;
            if (heightRate < 0.0) {
                rayleighColor = vec3(1.0, 0.0, 0.0);
                mieColor = vec3(1.0, 0.0, 0.0);
                break;
            }
            float cosSunlight = dot(normalizedSunLight, samplePoint) / pointHeight;
            float cosSight = dot(sightRay, samplePoint) / pointHeight;
            // vec3 tmp = findFromLUT(RLUT, heightRate, cosSight);
            // if (tmp.x <= 0.01) {
            //     rayleighColor = vec3(1.0, 0.0, 0.0);
            //     mieColor = vec3(1.0, 0.0, 0.0);
            //     break;
            // }
            vec3 RLUTTerm = cameraRTerm * findFromLUT(RLUT, heightRate, cosSunlight) / findFromLUT(RLUT, heightRate, cosSight);
            vec3 MLUTTerm = cameraMTerm * findFromLUT(MLUT, heightRate, cosSunlight) / findFromLUT(MLUT, heightRate, cosSight);
            rayleighColor += RLUTTerm * MLUTTerm * exp(-heightRate / rayleighBaseRate);

            mieColor += RLUTTerm * MLUTTerm * exp(-heightRate / mieBaseRate);

            samplePoint += deltaRay;
        }
    }
    else {
        float cosCamera = -dot(cameraPos, sightRay) / cameraHeight;
        float cameraHeightRate = max(0.0, (cameraHeight - earthRadius) / atmosphereThickness);
        vec3 cameraRTerm = findFromLUT(RLUT, cameraHeightRate, cosCamera);
        vec3 cameraMTerm = findFromLUT(MLUT, cameraHeightRate, cosCamera);
        for (int i = 0; i < samples; i++) {
            float pointHeight = length(samplePoint);;
            float heightRate = (pointHeight - earthRadius) / atmosphereThickness;
            if (heightRate < 0.0) {
                rayleighColor = vec3(1.0, 0.0, 0.0);
                mieColor = vec3(1.0, 0.0, 0.0);
                break;
            }
            float cosSunlight = dot(normalizedSunLight, samplePoint) / pointHeight;
            float cosSight = -dot(sightRay, samplePoint) / pointHeight;
            // vec3 tmp = findFromLUT(RLUT, heightRate, cosSight);
            // if (tmp.x <= 0.01) {
            //     rayleighColor = vec3(1.0, 0.0, 0.0);
            //     mieColor = vec3(1.0, 0.0, 0.0);
            //     break;
            // }
            vec3 RLUTTerm = findFromLUT(RLUT, heightRate, cosSight) * findFromLUT(RLUT, heightRate, cosSunlight) / cameraRTerm;
            vec3 MLUTTerm = findFromLUT(MLUT, heightRate, cosSight) * findFromLUT(MLUT, heightRate, cosSunlight) / cameraMTerm;;
            rayleighColor += RLUTTerm * MLUTTerm * exp(-heightRate / rayleighBaseRate);

            mieColor += RLUTTerm * MLUTTerm * exp(-heightRate / mieBaseRate);

            samplePoint += deltaRay;
        }
    }

    //rayleighResult = rayleighColor;
    vec3 rayleighResult = rayleighColor * (rayLength / samples) * rayleighPhaseTerm * rayleighTerm;
    vec3 mieResult = mieColor * (rayLength / samples) * miePhaseTerm * mieTerm;
    //FragColor = vec4((rayleighResult + mieResult) * lightColor, 1.0);
    FragColor = vec4(1.0 - exp(-1.5 * (rayleighResult + mieResult) * lightColor), 1.0);
}