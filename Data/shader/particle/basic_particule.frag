#version 330

layout(location=0) out vec4 outColor0;

uniform vec4 color;

flat in vec4 particleColor;

#ifdef USE_TEXTURE
uniform sampler2D texture0;
smooth in vec2 texCoord;
#endif

void main()
{
	outColor0 = particleColor*color;
	#ifdef USE_TEXTURE
	outColor0 *= texture(texture0, texCoord);
	#endif
}