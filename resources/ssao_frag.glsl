#version 450 core 
out vec4 color;
in vec2 fragTex;

uniform mat4 P;

// Sent from CPU using glActiveTexture(...)
uniform sampler2D texPos;
uniform sampler2D texNormal;
uniform sampler2D texNoise;

// Array size must match in CPU
uniform vec3 kernSamp[64];
int SAMPLESIZE = 64;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(640.0/4.0, 480.0/4.0);

void main()
{
	vec3 pos = texture(texPos, fragTex).rgb;
	vec3 normal = texture(texNormal, fragTex).rgb;
	vec3 randVec = texture(texNoise, fragTex * noiseScale).xyz; 
	
	// TBN - transform vector in tangent-space to view-space
	vec3 tangent   = normalize(randVec - normal * dot(randVec, normal));
	//vec3 tangent   = normalize(randVec - normal * normal);
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal); 

	float occlusion = 0.0;
	float radius = 0.15;
	//float radius = 1.0;
	for(int i = 0; i < SAMPLESIZE; ++i)
	{
		// get sample position
		vec3 sampPos = TBN * kernSamp[i]; // From tangent to view-space
		sampPos = pos + sampPos * radius; 
    
		// Transform samples from view-space to screen-space
		vec4 offset = vec4(sampPos, 1.0);
		offset      = P * offset;    // from view to clip-space
		offset.xyz /= offset.w;               // perspective divide
		offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0  

		// get the sample's depth
		float sampleDepth = texture(texPos, offset.xy).z; 

		float bias = 0.025;

		//float rangeCheck = smoothstep(0.0, 1.0, radius / abs(pos.z - sampleDepth));
		float rangeCheck= abs(pos.z - sampleDepth) < radius ? 1.0 : 0.0;
		occlusion       += (sampleDepth >= sampPos.z + bias ? 1.0 : 0.0) * rangeCheck;  
//occlusion       += (sampleDepth >= sampPos.z + bias ? 1.0 : 0.0);
	}
			
	
	occlusion = 1.0 - (occlusion / SAMPLESIZE);
	//occlusion = pow(occlusion, 2.0);

	//color.rgb = vec3(occlusion * occlusion);
	color.rgb = vec3(occlusion);
	color.a=1;
	//better results with HDR!
}
