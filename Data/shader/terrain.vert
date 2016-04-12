#version 430
#extension GL_ARB_bindless_texture : enable

invariant gl_Position;
  
in vec3 vertex;  
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

struct Material
{
	vec4 header_offset_size;
	uvec2 texture0;
	uvec2 texture1;
	uvec2 texture2;
	uvec2 texture3;
	
	vec4 scaleZ_sharp_reso_other;
};

layout(std140, binding = 2) uniform Materials
{
	Material materials[MAX_UBO_VEC4 / 4];
};

smooth out vec2 tCoord;
flat out int v_drawId;
smooth out vec3 v_normal;
smooth out vec3 v_tangent;
  
void main()  
{  
	v_drawId = drawId;
	gl_Position = models[drawId] * vec4(vertex,1);
	
	vec4 matInfo = materials[drawId].header_offset_size;
	tCoord = (gl_Position.xy+matInfo.yz) / vec2(matInfo.w);
	
	sampler2D hm = sampler2D(materials[drawId].texture0);
	//float dxy = 1.f / textureSize(hm, 0).x;
	
	const float SCALEZ = 100;
	const float SHARP_COEF = 50;
	const int O = textureSize(hm, 0).x / int(16*16*2);
	float f_px = textureOffset(hm, tCoord, ivec2(O,0)).r;
	float f_nx = textureOffset(hm, tCoord, ivec2(-O,0)).r;
	float f_py = textureOffset(hm, tCoord, ivec2(0,O)).r;
	float f_ny = textureOffset(hm, tCoord, ivec2(0,-O)).r;
	float f = textureOffset(hm, tCoord, ivec2(0,0)).r;
	
	vec2 sharpness = vec2(O,0) * matInfo.w / (SCALEZ*2*SHARP_COEF);
	
	vec3 va = normalize(vec3(sharpness.xy, f_px-f_nx));
    vec3 vb = normalize(vec3(sharpness.yx, f_py-f_ny));
    vec4 bump = vec4( cross(va,vb), f );
	v_normal = bump.xyz;
	
	
	mat3 nMat = mat3(models[drawId]);
	v_normal = nMat*v_normal;
	v_tangent = nMat*tangent;
	
	gl_Position.z += f*SCALEZ;
	gl_Position = projView * gl_Position;
}