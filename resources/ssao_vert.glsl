#version  450 core
layout(location = 0) in vec3 vertPos;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 fragNor;
out vec2 fragTex;
out vec3 fragPos;
out vec4 fragViewPos;

void main()
{
gl_Position = P * V * M * vertPos;
fragTex = vertTex;
}
