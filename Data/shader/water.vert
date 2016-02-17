#version 330 
#define INSTANCING 128 
invariant gl_Position; 
in vec3 vertex; 
in vec3 normal; 
in vec2 texCoord;
in vec3 tangent; 

smooth out vec3 in_normal;
smooth out vec3 in_tangent;
 
uniform mat4 projView; 
uniform mat4 model[INSTANCING]; 

smooth out vec3 vDir;
smooth out vec2 texCoord0;

void main() 
{ 
	vec4 vertDir = model[gl_InstanceID]*vec4(vertex,1); 
	gl_Position = projView * vertDir;
	vDir = vec3(vertDir);
	
	mat3 normalMat=mat3(model[gl_InstanceID]);
    #ifdef ACCURATE_NORMAL
        normalMat = transpose(inverse(normalMat));
    #endif
	
	in_normal = normalize(normalMat*normal);
	in_tangent = normalize(normalMat*tangent);
	
	texCoord0 = texCoord*2;
}