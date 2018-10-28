#version 450 core 
out vec4 color;
in vec2 fragTex;
uniform sampler2D texColor;


void main()
{

vec3 texturecolor = texture(texColor, fragTex).rgb;

	color.rgb = texturecolor * 1.5;
	color.a=1;
	//better results with HDR!
}
