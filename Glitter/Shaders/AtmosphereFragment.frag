#version 330 core
out vec4 FragColor;

in vec3 rayleighResult;

uniform vec3 lightColor;

void main() {
    
    //FragColor = vec4(rayleighResult * lightColor, 1.0);
    FragColor = vec4(1.0 - exp(-1.5 * rayleighResult * lightColor), 1.0);
}