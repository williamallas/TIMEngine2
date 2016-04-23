#version 430
#extension GL_ARB_bindless_texture : enable

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

smooth in vec2 tCoord;
flat in int v_drawId;
smooth in vec3 v_normal;
smooth in vec3 v_tangent;
  
layout(location=0) out vec4 outColor; 
layout(location=1) out vec4 outNormal;
layout(location=2) out vec4 outMaterial;
  
void main()  
{  	
	sampler2DArray colorTex = sampler2DArray(materials[v_drawId].texture1);
	sampler2D maskTex = sampler2D(materials[v_drawId].texture2);
	
	vec4 mask = texture(maskTex, tCoord);
	vec4 texColor = texture(colorTex, vec3(tCoord*materials[v_drawId].scales[0][0],0));
	
	texColor = texture(colorTex, vec3(tCoord*materials[v_drawId].scales[0][1],1)) * mask.x + texColor*(1-mask.x);
	
	//vec3 n = normalize(v_normal);
	
	outColor = texColor;
	
	//outColor = vec4(v_normal*0.5+0.5,1);
	outNormal = vec4(v_normal*0.5+0.5,1);
	outMaterial = vec4(1,0,0.5,1);
}