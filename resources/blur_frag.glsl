#version 450 core 
out vec4 color;
in vec2 fragTex;

// Sent from CPU using glActiveTexture(...)
uniform sampler2D texSSAO;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(texSSAO, 0));
    float result = 0.0;
    for (int x = 0; x < 4; ++x) 
    {
        for (int y = 0; y < 4; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(texSSAO, fragTex + offset).r;
        }
    }

    color.rgb = vec3(result / (4.0 * 4.0));
	color.a = 1.0;

	//color = texture(texSSAO, fragTex);
}  