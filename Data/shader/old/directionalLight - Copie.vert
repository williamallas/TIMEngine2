#version 430

precision highp float;

invariant gl_Position;  
in vec3 vertex;  
in vec2 texCoord;

out vec2 coord;

void main()  
{
	coord = texCoord;	
	gl_Position=vec4(vertex,1);
}