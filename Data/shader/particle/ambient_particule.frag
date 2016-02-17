#version 330

layout(location=0) out vec4 outColor0;

uniform vec4 color;

uniform sampler2D texture0;

smooth in vec2 texCoord;

void main()
{
	outColor0 = color * texture(texture0, texCoord);
}