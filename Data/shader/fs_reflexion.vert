#version 430

in vec3 vertex;
in vec2 texCoord;

smooth out vec2 texc;

void main()
{
	texc = texCoord;
	gl_Position = vec4(vertex,1);
}