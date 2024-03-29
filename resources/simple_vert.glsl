#version  450 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
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
fragPos= (M * vec4(vertPos, 1.0)).xyz;
fragViewPos = V * M * vec4(vertPos, 1.0);
gl_Position = P * V * M * vec4(vertPos, 1.0);
fragNor = (V * M * vec4(vertNor, 0.0)).xyz;
fragTex = vertTex;
fragTex.y*=-1;
}
