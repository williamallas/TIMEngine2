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

#ifdef PORTAL_SHADER
struct Material
{
	uvec2 header;
	uvec2 tex0;
	uvec2 tex1;
	uvec2 tex2;
	vec4 parameter;
	uvec4 color_scale_ca_unsused;
};

layout(std140, binding = 2) uniform Materials
{
	Material materials[MAX_UBO_VEC4 / 4];
};
#endif

uniform vec4 clipPlan0;

smooth out vec2 tCoord;
flat out int v_drawId;
smooth out vec3 v_normal;
smooth out vec3 v_tangent;
  
void main()  
{  
	tCoord = texCoord;
	v_drawId = drawId;
	
#ifdef PORTAL_SHADER
	vec4 worldVert = models[drawId] * vec4(vertex,1);
	float dist = dot(worldVert, materials[drawId].parameter);
	
	if(dist < 0)
		gl_Position = vec4(cameraPos.xyz,0); // culled out
	else
		gl_Position = projView * worldVert;
	
	v_normal = vec3(0,0,0);
	v_tangent = vec3(0,0,0);
#else
	vec4 worldVert = models[drawId] * vec4(vertex,1);
	gl_ClipDistance[0] = dot(worldVert, clipPlan0);
	gl_Position = projView * worldVert;
	
	mat3 nMat = mat3(models[drawId]);
	v_normal = nMat*normal;
	v_tangent = nMat*tangent;
#endif
	
	
}