#version 430
#extension GL_ARB_bindless_texture : enable

struct Material
{
	uvec2 header;
	uvec2 tex0;
	uvec2 tex1;
	uvec2 tex2;
	vec4 parameter;
	vec4 color;
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
	vec4 texColor = vec4(1);
	vec3 n = normalize(v_normal);
	
	if(materials[v_drawId].header.y > 0)
	{
		texColor = texture(sampler2D(materials[v_drawId].tex0), tCoord);
		if(materials[v_drawId].header.y > 1)
		{
			vec3 t = normalize(v_tangent);
			mat3 tbn = mat3(t, cross(t,n), n);
			n = tbn*(texture(sampler2D(materials[v_drawId].tex1), tCoord).xyz*2-1);
		}
	}
	
	outColor = texColor * materials[v_drawId].color;
	outNormal = vec4(n*0.5+0.5,1);
	outMaterial = vec4(materials[v_drawId].parameter);
}