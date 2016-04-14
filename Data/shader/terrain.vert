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

	uvec2 texture0;
	uvec2 texture1;
	uvec2 texture2;
	uvec2 texture3;
	
	vec4 scales[2];
};

layout(std140, binding = 2) uniform Materials
{
	Material materials[MAX_UBO_VEC4 / 4];
};

layout(std140, binding = 3) uniform TerrainInfos
{
	vec4 offset_zscale_sharpness;
	vec4 XYsize_vRes;
};

smooth out vec2 tCoord;
flat out int v_drawId;
smooth out vec3 v_normal;
smooth out vec3 v_tangent;
  
void main()  
{  
	vec2 OFFSET = offset_zscale_sharpness.xy;
	float SCALEZ = offset_zscale_sharpness.z;
	float SHARPNESS = offset_zscale_sharpness.w;
	float TRES = XYsize_vRes.y;
	float TERRAIN_SIZE = XYsize_vRes.x;
	v_drawId = drawId;
	gl_Position = models[drawId] * vec4(vertex,1);
	
	tCoord = (gl_Position.xy+OFFSET) / vec2(TERRAIN_SIZE);
	
	sampler2D hm = sampler2D(materials[drawId].texture0);
	//float dxy = 1.f / textureSize(hm, 0).x;
	
	const int O = textureSize(hm, 0).x / int(TRES*2);
	float f_px = textureOffset(hm, tCoord, ivec2(O,0)).r;
	float f_nx = textureOffset(hm, tCoord, ivec2(-O,0)).r;
	float f_py = textureOffset(hm, tCoord, ivec2(0,O)).r;
	float f_ny = textureOffset(hm, tCoord, ivec2(0,-O)).r;
	float f = textureOffset(hm, tCoord, ivec2(0,0)).r;
	
	/*float f_px = texture(hm, tCoord + vec2(O*dxy,0)).r;
	float f_nx = texture(hm, tCoord + vec2(-O*dxy,0)).r;
	float f_py = texture(hm, tCoord + vec2(0,O*dxy)).r;
	float f_ny = texture(hm, tCoord + vec2(0,-O*dxy)).r;
	float f = texture(hm, tCoord + ivec2(0,0)).r;*/
	
	vec2 sharpness = vec2(O,0) * TERRAIN_SIZE / (SCALEZ*2*SHARPNESS);
	
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