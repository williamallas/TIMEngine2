#version 430

invariant gl_Position;
  
in vec3 vertex;  
in vec3 normal;
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

smooth out vec3 v_normal;

float pixelWidthRatio = 2.f / (1000 * proj[0][0]);
  
void main()  
{  
	vec3 vWorld = vec3(models[drawId] * vec4(vertex,1));
	
	mat3 nMat = mat3(models[drawId]);
	v_normal = nMat*normal;
	
	gl_Position = projView * vec4(vWorld,1);
	float pixelSize = gl_Position.w * pixelWidthRatio;
	
	gl_Position = projView * vec4(vWorld + normalize(v_normal)*pixelSize*1.75,1);

	//gl_Position.z += 0.001;
}