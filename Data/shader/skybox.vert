#version 330 
#define INSTANCING 128 
invariant gl_Position; 
in vec3 vertex; 
in vec2 texCoord0;
 
uniform mat4 projView; 
uniform mat4 model[INSTANCING]; 

smooth out vec3 vDir;
smooth out vec2 texCoord;

void main() 
{ 
	mat4 mat = projView*model[gl_InstanceID]; 
	gl_Position=mat*vec4(vertex,1); 
	
	vDir = normalize(vertex);
	texCoord = vDir.xz *0.5+0.5; //texCoord0;
}