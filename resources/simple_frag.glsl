#version 450 core 

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 viewpos;
layout(location = 2) out vec4 gNormal;

in vec3 fragPos;
in vec2 fragTex;
in vec3 fragNor;
in vec4 fragViewPos;

uniform vec3 campos;

layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform sampler2D tex2;


void main()
{
	vec3 normal = normalize(fragNor);
	vec3 texturecolor = texture(tex, fragTex).rgb;
	
	color.rgb = texturecolor;
	color.a=1;
	viewpos = fragViewPos;
	//viewpos.z*=-1;
	gNormal = vec4(normal, 1.0);
}
