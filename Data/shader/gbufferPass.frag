#version 430
#extension GL_ARB_bindless_texture : enable

struct Material
{
	uvec2 header;
	uvec2 tex0;
	uvec2 tex1;
	uvec2 tex2;
	vec4 parameter;
	uvec4 color_scale_ca_unsused; // ca == cubeMapAffected
};

#ifdef WATER_SHADER
layout(std140, binding = 0) uniform DrawParameter
{
	mat4 view, proj;
	mat4 projView, invView, invProj, invViewProj, worldOriginInvViewProj;
	vec4 cameraPos, cameraUp, cameraDir, worldOrigin;
	vec4 time;
}; 
#endif

vec4 unpackColor(uint col)
{
	uint r_i = (col & 0xff000000)  >> 24;
    uint g_i = (col & 0x00ff0000)  >> 16;
    uint b_i = (col & 0x0000ff00)  >> 8;
    uint a_i = (col & 0x000000ff);
	vec4 rgba;
    rgba.r = r_i / 255.f;
    rgba.g = g_i / 255.f;
    rgba.b = b_i / 255.f;
    rgba.a = a_i / 255.f;
	return rgba;
}


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
	vec3 n = v_normal;
	vec4 material_tex = vec4(1,1,1,1);
	
	if(materials[v_drawId].header.y > 0)
	{
		float texScale = materials[v_drawId].color_scale_ca_unsused.y / 1000.f;
		texColor = texture(sampler2D(materials[v_drawId].tex0), tCoord * texScale);
		
	#ifdef ALPHA_TEST
		if(texColor.a < 0.5) discard;
	#endif
	
		if(materials[v_drawId].header.y > 1)
		{
		#ifdef WATER_SHADER
			vec2 dirtex = (materials[v_drawId].parameter.xy-vec2(0.5))*4;
			vec2 dirtex2 = vec2(cos(dirtex.y), sin(dirtex.y));
			dirtex = vec2(cos(dirtex.x), sin(dirtex.x));
			
			float scaleTime = materials[v_drawId].parameter.z * 0.05;
			vec3 n1 = texture(sampler2D(materials[v_drawId].tex1), (tCoord * texScale * 20 + dirtex*time.x*scaleTime)).xyz*2-1;
			vec3 n2 = texture(sampler2D(materials[v_drawId].tex1), (tCoord * texScale * 20 + dirtex2*time.x*scaleTime)).xyz*2-1;
			n = (n1+n2) * 0.5;
			n.z *= materials[v_drawId].parameter.w * 10;
		#else
			vec3 t = normalize(v_tangent);
			mat3 tbn = mat3(t, cross(t,n), n);
			n = tbn*(texture(sampler2D(materials[v_drawId].tex1), tCoord * texScale).xyz*2-1);
			if(materials[v_drawId].header.y > 2)
			{
				material_tex = texture(sampler2D(materials[v_drawId].tex2), tCoord * texScale);
			}
		#endif
		}
	}
	n = normalize(n);
	
	outColor = texColor * unpackColor(materials[v_drawId].color_scale_ca_unsused.x);
	outNormal = vec4(n*0.5+0.5, 1);
	#ifdef PORTAL_SHADER
	outMaterial = vec4(1, 1, materials[v_drawId].color_scale_ca_unsused.y / 1000.f,0);
	#else
	#ifdef WATER_SHADER
	outMaterial = vec4(0, 1, 1,0);
	#else
	outMaterial = vec4(materials[v_drawId].parameter) * material_tex;
	int spec = int(outMaterial.z * 255.f + 0.5);
	spec = (spec >> 1) << 1;
	spec += materials[v_drawId].color_scale_ca_unsused.z>0 ? 1:0;
	outMaterial.z = spec / 255.f;
	#endif
	#endif
}