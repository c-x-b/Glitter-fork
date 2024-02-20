// #include<Header.glsl>

// set in render
uniform mat4 model;

// debug
uniform bool mode;
uniform float renderBoundary;

in vec3 actualPos;

out vec4 FragColor;

void main() {
    vec3 outPos = normalize(actualPos) * atmosphereRadius;

    vec3 sightRay = outPos - cameraPos;
    float rayLength = length(sightRay); // length of the light ray in atmosphere
    sightRay /= rayLength; // normalized

    vec3 startPos;

    // float B = 2.0 * dot(cameraPos, sightRay);
	// float C = cameraHeight2 - atmosphereRadius2;
	// float fDet = max(0.0, B * B - 4.0 * C);
	// float fNear = 0.5 * (-B - sqrt(fDet));

    float tmp = dot(-cameraPos, sightRay);
    float halfChord = rayLength - tmp;
    float rayHeight = sqrt(atmosphereRadius2 - halfChord * halfChord);
    startPos = cameraPos;

    float atmosphereThickness = atmosphereRadius - earthRadius;
    float boundaryHeight = earthRadius + renderBoundary * atmosphereThickness;

    vec3 rayleighResult;
    vec3 mieResult;
    
    float temp = dot(outPos, cameraPos) / cameraHeight; 
    if (temp >= boundaryHeight) {
        raytraceScatterAtmosForwardInside(rayleighResult, mieResult, startPos, sightRay, rayLength);
    }
    else {
        raytraceScatterAtmosBackwardInside(rayleighResult, mieResult, startPos, sightRay, rayLength);
    }

    //FragColor = vec4((rayleighResult + mieResult) * lightColor, 1.0);
    FragColor = vec4(1.0 - exp(-1.5 * (rayleighResult + mieResult) * lightColor), 1.0);
}