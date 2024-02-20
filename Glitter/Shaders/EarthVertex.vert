// #include<Header.glsl>

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out float cameraDistance;
out vec3 scatteringResult;
out vec3 groundAttenuate;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    vec3 scaledPos = normalize(FragPos) * earthRadius;
    vec3 sightRay = scaledPos - cameraPos;
    float rayLength = length(sightRay);
    sightRay /= rayLength; // normalized

    vec3 startPos;
    vec3 rayleighResult, mieResult;
    if (cameraHeight > atmosphereRadius)
    {
        float tmp = dot(-cameraPos, sightRay);
        float rayHeight2 = cameraHeight2 - tmp * tmp;
        float halfChord = sqrt(atmosphereRadius2 - rayHeight2);
        startPos = cameraPos + (tmp - halfChord) * sightRay; 
        rayLength -= tmp - halfChord;
        raytraceScatterGroundOutside(rayleighResult, mieResult, groundAttenuate, startPos, sightRay, rayLength); 
    }
    else
    {
        startPos = cameraPos;
        // rayleighResult = vec3(0.0);
        // mieResult = vec3(0.0);
        // groundAttenuate = vec3(0.0);
        raytraceScatterGroundInside(rayleighResult, mieResult, groundAttenuate, startPos, sightRay, rayLength);
    }

    cameraDistance = clamp(abs(atmosphereRadius - cameraHeight) / 3.0, 0.1, 0.5);
    scatteringResult = vec3(1.0) - exp(-1.5 * (rayleighResult) * lightColor);

    Normal = mat3(transpose(inverse(model))) * aNormal;  
    gl_Position = projection * view * vec4(FragPos, 1.0);
	TexCoords = aTexCoords;
}