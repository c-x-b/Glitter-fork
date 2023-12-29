#version 330 core
precision highp float;
out vec4 FragColor;

in vec3 rayleighResult;
in vec3 mieResult;

uniform vec3 lightColor;

void main() {
    //FragColor = vec4(rayleighResult, 1.0);
    FragColor = vec4((rayleighResult) * lightColor, 1.0);
    //FragColor = vec4(1.0 - exp(-1.5 * (rayleighResult + mieResult) * lightColor), 1.0);
}