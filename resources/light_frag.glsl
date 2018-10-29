#version 450 core 
out vec4 color;

in vec2 fragTex;

uniform sampler2D texColor;
uniform sampler2D texPos;
uniform sampler2D texNormal;
uniform sampler2D texBlurredSSAO;

uniform vec3 campos;

void main()
{
	vec3 fragCol = texture(texColor, fragTex).rgb;
	vec3 fragNor = texture(texNormal, fragTex).rgb;
	vec3 fragPos = texture(texPos, fragTex).rgb;
	vec3 fragSSAO = texture(texBlurredSSAO, fragTex).rgb;

	//diffuse light

	vec3 ambient = vec3(fragCol * fragSSAO); // here we add occlusion factor
	vec3 lp = vec3(0, 0, -0.5);
	vec3 ld = normalize(lp - fragPos);
	float light = dot(ld,fragNor);	
	light = clamp(light,0,1);

	//specular light
	vec3 camvec = normalize(campos - fragPos);
	vec3 h = normalize(camvec+ld);
	float spec = pow(dot(h,fragNor),5);
	spec = clamp(spec,0,1)*0.1;
	
	color.rgb = ambient *light + vec3(1,1,1)*spec;
	//color.rgb = fragCol * fragSSAO * light + vec3(1)*spec;
	//color.rgb *= 2.0;

	//color.rgb = fragSSAO;
	color.rgb = ambient * light;// + vec3(1) * spec;
	color.rgb = fragSSAO;// * light;
	color.a=1;
}
