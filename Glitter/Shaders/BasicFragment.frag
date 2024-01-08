#version 330 core
precision highp float;
out vec4 FragColor;

in vec2 TexCoord;
//in vec3 actualPos;

uniform vec3 cameraPos;
uniform sampler2DRect texture1;

void main()
{
    FragColor = texture(texture1, TexCoord * 500.0);
    //FragColor = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0);
} 