#version 330 core
precision highp float;
precision highp int;

// set in main
uniform vec3 cameraPos;
uniform float cameraHeight;
uniform float cameraHeight2;
uniform vec3 sunLightDir; // normalized
uniform float atmosphereRadius;
uniform float atmosphereRadius2;
uniform float earthRadius;
uniform float const3Divide16PI; // 3/(16PI)
uniform float PI;
uniform vec3 lightColor;

uniform sampler2DRect LUT;
uniform vec3 rayleighTerm; // β(λ)
uniform vec3 mieTerm;
uniform int samples;
uniform int LUTTableSize;
uniform float rayleighBaseRate;
uniform float mieBaseRate;
uniform float g;
uniform float g2; // g*g

float calcRayleighPhase(float cosSunAndSight)
{
    return (1.0 + cosSunAndSight * cosSunAndSight) * const3Divide16PI;
}

float calcMiePhase(float cosSunAndSight)
{
    return (1.0 - g2) * (1.0 + cosSunAndSight * cosSunAndSight) / ((2 + g2) * pow(1 + g2 - 2 * g * cosSunAndSight, 1.5)) * 2 * const3Divide16PI;
}


vec3 findFromLUT(sampler2DRect LUT, float height, float cosValue) 
{
    float angle = acos(cosValue);
    float normalizedAngle = angle / PI;
    //vec4 findResult = texelFetch(LUT, ivec2(normalizedAngle * LUTTableSize, height * LUTTableSize), 0);
    vec4 findResult = texture(LUT, vec2(normalizedAngle * LUTTableSize, height * LUTTableSize));
    return findResult.rgb;
}

void raytraceScatterAtmosOutside(
        out vec3 rayleighResult,
        out vec3 mieResult,
        vec3 startPos,
        vec3 sightRay,
        float rayLength)
{
    float atmosphereThickness = atmosphereRadius - earthRadius;
    vec3 deltaRay = rayLength / samples * sightRay;
    vec3 samplePoint = startPos + deltaRay * 0.5; 
    vec3 rayleighColor = vec3(0.0);
    vec3 mieColor = vec3(0.0);
    
    for (int i = 0; i < samples; i++) {
        float pointHeight = length(samplePoint);
        float heightRate = max(0.0, (pointHeight - earthRadius) / atmosphereThickness);
        float cosSight = -dot(sightRay, samplePoint) / pointHeight;
        float cosSunlight = dot(sunLightDir, samplePoint) / pointHeight;
        vec3 LUTTerm = findFromLUT(LUT, heightRate, cosSight) * findFromLUT(LUT, heightRate, cosSunlight);
        rayleighColor += LUTTerm * exp(-heightRate / rayleighBaseRate);

        mieColor += LUTTerm * exp(-heightRate / mieBaseRate);

        samplePoint += deltaRay;
    }

    float cosSunAndSight = dot(sunLightDir, sightRay);
    rayleighResult = rayleighColor * (rayLength / samples) * calcRayleighPhase(cosSunAndSight) * rayleighTerm;
    mieResult = mieColor * (rayLength / samples) * calcMiePhase(cosSunAndSight) * mieTerm;
}

/* A     B      C         D
   <-----*------*---------*
   A: Intersection with atmosphere forward
   B: Sampled point
   C: Camera point
   D: Intersection with atmosphere backward
   
   We need to calc optical depth of BC, using CA/BA or BD/CD, 
   all of which could be found using LUT
*/

// using CA/BA
void raytraceScatterAtmosForwardInside(
        out vec3 rayleighResult,
        out vec3 mieResult,
        vec3 startPos,
        vec3 sightRay,
        float rayLength)
{
    float atmosphereThickness = atmosphereRadius - earthRadius;
    vec3 deltaRay = rayLength / samples * sightRay;
    vec3 samplePoint = startPos + deltaRay * 0.5; 
    vec3 rayleighColor = vec3(0.0);
    vec3 mieColor = vec3(0.0);

    float cosCamera = dot(cameraPos, sightRay) / cameraHeight;
    float cameraHeightRate = max(0.0, (cameraHeight - earthRadius) / atmosphereThickness);
    vec3 cameraTerm = findFromLUT(LUT, cameraHeightRate, cosCamera);
    for (int i = 0; i < samples; i++) {
        float pointHeight = length(samplePoint);;
        float heightRate = (pointHeight - earthRadius) / atmosphereThickness;
        if (heightRate < 0.0) {
            rayleighColor = vec3(1.0, 0.0, 0.0);
            mieColor = vec3(1.0, 0.0, 0.0);
            break;
        }
        float cosSunlight = dot(sunLightDir, samplePoint) / pointHeight;
        float cosSight = dot(sightRay, samplePoint) / pointHeight;
        vec3 LUTTerm = cameraTerm * findFromLUT(LUT, heightRate, cosSunlight) / findFromLUT(LUT, heightRate, cosSight);
        
        rayleighColor += LUTTerm * exp(-heightRate / rayleighBaseRate);
        mieColor += LUTTerm * exp(-heightRate / mieBaseRate);

        samplePoint += deltaRay;
    }

    float cosSunAndSight = dot(sunLightDir, sightRay);
    rayleighResult = rayleighColor * (rayLength / samples) * calcRayleighPhase(cosSunAndSight) * rayleighTerm;
    mieResult = mieColor * (rayLength / samples) * calcMiePhase(cosSunAndSight) * mieTerm;
}

// using BD/CD
void raytraceScatterAtmosBackwardInside(
        out vec3 rayleighResult,
        out vec3 mieResult,
        vec3 startPos,
        vec3 sightRay,
        float rayLength)
{
    float atmosphereThickness = atmosphereRadius - earthRadius;
    vec3 deltaRay = rayLength / samples * sightRay;
    vec3 samplePoint = startPos + deltaRay * 0.5; 
    vec3 rayleighColor = vec3(0.0);
    vec3 mieColor = vec3(0.0);

    float cosCamera = -dot(cameraPos, sightRay) / cameraHeight;
    float cameraHeightRate = max(0.0, (cameraHeight - earthRadius) / atmosphereThickness);
    vec3 cameraTerm = findFromLUT(LUT, cameraHeightRate, cosCamera);
    for (int i = 0; i < samples; i++) {
        float pointHeight = length(samplePoint);;
        float heightRate = (pointHeight - earthRadius) / atmosphereThickness;
        if (heightRate < 0.0) {
            rayleighColor = vec3(1.0, 0.0, 0.0);
            mieColor = vec3(1.0, 0.0, 0.0);
            break;
        }
        float cosSunlight = dot(sunLightDir, samplePoint) / pointHeight;
        float cosSight = -dot(sightRay, samplePoint) / pointHeight;
        vec3 LUTTerm = findFromLUT(LUT, heightRate, cosSight) * findFromLUT(LUT, heightRate, cosSunlight) / cameraTerm;
        
        rayleighColor += LUTTerm * exp(-heightRate / rayleighBaseRate);
        mieColor += LUTTerm * exp(-heightRate / mieBaseRate);

        samplePoint += deltaRay;
    }

    float cosSunAndSight = dot(sunLightDir, sightRay);
    rayleighResult = rayleighColor * (rayLength / samples) * calcRayleighPhase(cosSunAndSight) * rayleighTerm;
    mieResult = mieColor * (rayLength / samples) * calcMiePhase(cosSunAndSight) * mieTerm;
}

void raytraceScatterGroundOutside(
    out vec3 rayleighResult,
    out vec3 mieResult,
    out vec3 groundAttenuate,
    vec3 startPos,
    vec3 sightRay,
    float rayLength
)
{
    float atmosphereThickness = atmosphereRadius - earthRadius;
    vec3 deltaRay = rayLength / samples * sightRay;
    vec3 samplePoint = startPos + deltaRay * 0.5; 
    vec3 rayleighColor = vec3(0.0);
    vec3 mieColor = vec3(0.0);
    
    vec3 LUTTerm;
    for (int i = 0; i < samples; i++) {
        float pointHeight = length(samplePoint);
        float heightRate = max(0.0, (pointHeight - earthRadius) / atmosphereThickness);
        float cosSight = -dot(sightRay, samplePoint) / pointHeight;
        float cosSunlight = dot(sunLightDir, samplePoint) / pointHeight;
        LUTTerm = findFromLUT(LUT, heightRate, cosSight) * findFromLUT(LUT, heightRate, cosSunlight);
        rayleighColor += LUTTerm * exp(-heightRate / rayleighBaseRate);

        mieColor += LUTTerm * exp(-heightRate / mieBaseRate);

        samplePoint += deltaRay;
    }

    float cosSunAndSight = dot(sunLightDir, sightRay);
    rayleighResult = rayleighColor * (rayLength / samples) * calcRayleighPhase(cosSunAndSight) * rayleighTerm;
    mieResult = mieColor * (rayLength / samples) * calcMiePhase(cosSunAndSight) * mieTerm;
    groundAttenuate = LUTTerm;
}

void raytraceScatterGroundInside(
    out vec3 rayleighResult,
    out vec3 mieResult,
    out vec3 groundAttenuate,
    vec3 startPos,
    vec3 sightRay,
    float rayLength
)
{
    float atmosphereThickness = atmosphereRadius - earthRadius;
    vec3 deltaRay = rayLength / samples * sightRay;
    vec3 samplePoint = startPos + deltaRay * 0.5; 
    vec3 rayleighColor = vec3(0.0);
    vec3 mieColor = vec3(0.0);
    
    vec3 LUTTerm;
    float cosCamera = -dot(cameraPos, sightRay) / cameraHeight;
    float cameraHeightRate = max(0.0, (cameraHeight - earthRadius) / atmosphereThickness);
    vec3 cameraTerm = findFromLUT(LUT, cameraHeightRate, cosCamera);
    for (int i = 0; i < samples; i++) {
        float pointHeight = length(samplePoint);
        float heightRate = max(0.0, (pointHeight - earthRadius) / atmosphereThickness);
        float cosSight = -dot(sightRay, samplePoint) / pointHeight;
        float cosSunlight = dot(sunLightDir, samplePoint) / pointHeight;
        LUTTerm = findFromLUT(LUT, heightRate, cosSight) * findFromLUT(LUT, heightRate, cosSunlight) / cameraTerm;
        rayleighColor += LUTTerm * exp(-heightRate / rayleighBaseRate);

        mieColor += LUTTerm * exp(-heightRate / mieBaseRate);

        samplePoint += deltaRay;
    }

    float cosSunAndSight = dot(sunLightDir, sightRay);
    rayleighResult = rayleighColor * (rayLength / samples) * calcRayleighPhase(cosSunAndSight) * rayleighTerm;
    mieResult = mieColor * (rayLength / samples) * calcMiePhase(cosSunAndSight) * mieTerm;
    groundAttenuate = LUTTerm;
}