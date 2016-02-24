#version 430

struct Material
{
	vec4 id_texture1;
	vec4 texures23;
	vec4 parameter;
	vec4 color;
};

layout(std140, binding = 2) uniform Materials
{
	Material materials[MAX_UBO_VEC4 / 4];
};

flat in int v_drawId;
smooth in vec3 v_normal;
  
layout(location=0) out vec4 outColor; 
layout(location=1) out vec4 outNormal;
layout(location=2) out vec4 outMaterial;
  
void main()  
{  	
	outColor = materials[v_drawId].color;
	outNormal = vec4(v_normal*0.5+0.5,1);
	outMaterial = vec4(materials[v_drawId].parameter);
}