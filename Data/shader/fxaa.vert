#version 430

in vec3 vertex;
in vec2 texCoord;

smooth out vec2 texc;
smooth out vec4 posPos;

uniform float FXAA_SUBPIX_SHIFT = 1.0/8.0;

uniform sampler2D texture0;

void main()
{
	texc = texCoord;
	gl_Position = vec4(vertex,1);
	
	ivec2 texSize = textureSize(texture0, 0);
	vec2 rcpFrame = vec2(1.0/texSize.x, 1.0/texSize.y);
	
	posPos.xy = texc;
	posPos.zw = texc - (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT));
}