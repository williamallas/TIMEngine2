#version 430

invariant gl_Position;
  
in vec3 vertex;  
in vec3 normal; 
in vec2 texCoord;
in vec3 tangent;
in int drawId;

layout(std140, binding = 0) uniform DrawParameter
{
	mat4 view, proj;
	mat4 projView, invView, invProj, invViewProj, worldOriginInvViewProj;
	vec4 cameraPos, cameraUp, cameraDir, worldOrigin;
	vec4 time;
}; 

layout(std140, binding = 1) uniform ModelMatrix
{
	mat4 models[MAX_UBO_VEC4 / 4];
};

flat out int v_drawId;
smooth out vec3 v_normal;
  
void main()  
{  
	v_drawId = drawId;
	gl_Position = projView * models[drawId] * vec4(vertex,1);
	v_normal = mat3(models[drawId])*normal;
}