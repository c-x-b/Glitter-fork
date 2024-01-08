#version 330 core
precision highp float;
layout (location = 0) in vec3 aPos;

// set in main
uniform mat4 view;
uniform mat4 projection;

// set in render
uniform mat4 model;

out vec3 actualPos;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    actualPos = vec3(model * vec4(aPos, 1.0));
}