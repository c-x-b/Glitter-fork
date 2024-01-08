#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
//out vec3 actualPos;

// uniform mat4 model;
//uniform mat4 view_T;
//uniform mat4 projection_T;

void main()
{
    //gl_Position =   projection * view * model * vec4(aPos, 1.0);
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    //vec4 tmpPos = view_T * projection_T * vec4(aPos.x, aPos.y, -1.0, 1.0);
    //actualPos = (tmpPos / tmpPos.w).xyz;
    TexCoord = aTexCoord;
}