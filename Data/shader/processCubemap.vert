#version 430

precision highp float;

in vec3 vertex;  

out vec2 coord;

void main()  
{
	coord = vertex.xy;	
	gl_Position=vec4(vertex,1);
}