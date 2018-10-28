#version 450 core 
out vec4 color;
in vec2 fragTex;

uniform mat4 P;

// Sent from CPU using glActiveTexture(...)
uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D noiseTex;

// Array size must match in CPU
uniform vec3 kernSamp[64];
int SAMPLESIZE = 64;


float CosInterpolate(float v1, float v2, float a)
	{
	float angle = a * 3.1415926;
	float prc = (1.0f - cos(angle)) * 0.5f;
	return  v1*(1.0f - prc) + v2*prc;
	}
vec2 calc_depth_fact(vec2 texcoords)
	{
	float depth = texture(gPos, texcoords).b;
	//some number magic:
	float processedDepthFact = depth/7.0;
	processedDepthFact = CosInterpolate(0,5,processedDepthFact);
	processedDepthFact = pow(processedDepthFact,2);
	return vec2(depth,processedDepthFact);
	}

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(1280.0/4.0, 720.0/4.0);

void main()
{
	vec3 pos = texture(gPos, fragTex).rgb;
	vec3 normal = texture(gNormal, fragTex).rgb;
	vec3 randVec = texture(noiseTex, fragTex * noiseScale).xyz;  
	vec2 depthfact = calc_depth_fact(fragTex);
	
	// TBN - transform vector in tangent-space to view-space
	vec3 tangent   = normalize(randVec - normal * dot(randVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal); 

	float occlusion = 0.0;
	float radius = 0.5;
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
		float sampleDepth = texture(gPos, offset.xy).z; 

		float bias = 0.025;

//		if (sampleDepth > pos.z + bias) {
//			occlusion += sampleDepth + bias;
//		}
		occlusion += (sampleDepth >= sampPos.z + bias ? 1.0 : 0.0);  
	}
			
	
	occlusion = 1.0 - (occlusion / SAMPLESIZE);

	//color.rgb = texturecolor*0.8;
	color.rgb = vec3(occlusion)*0.8;
	color.a=1;
	//better results with HDR!
}
