#version 430
 
precision highp float;
const float PI = 3.14159265359;

//uniform sampler2DArray textures[8];
//uniform sampler2DArrayShadow textures[8];
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2DArrayShadow textures[4];

smooth in vec2 coord;
 
layout(location=0) out vec4 outColor; 

layout(std140, binding = 0) uniform DrawParameter
{
	mat4 view, proj;
	mat4 projView, invView, invProj, invProjView, worldOriginInvProjView;
	vec4 cameraPos, cameraUp, cameraDir, worldOrigin;
	vec4 time;
}; 

#define MAX_LIGHTS 8
#define MAX_SHADOW_LVL 4
uniform int nbLights;
uniform vec4 lightDirection[MAX_LIGHTS];
uniform vec4 lightColor[MAX_LIGHTS];
uniform mat4 lightMatrix[MAX_LIGHTS * MAX_SHADOW_LVL];

vec3 computeFragPos(float depth, vec2 texCoord)
{
	vec4 position = invProjView * vec4((texCoord.st-vec2(0.5))*2.0f, (depth-0.5)*2.0f, 1);
	return position.xyz / position.w;
}

float D_GGX(float rough, float dotNH)
{
	float rough2 = rough*rough;
	float alpha2 = rough2 * rough2;
	float deno = dotNH*dotNH*(alpha2-1)+1;
	return alpha2 / (PI*deno*deno);
}

float helper_G_smith(float dotVal, float k)
{
	return dotVal / (dotVal*(1-k)+k);
}

float G_smith(float dotNV, float dotNL, float roughness)
{
	float k = (roughness+1)*(roughness+1) / 8;
    return helper_G_smith(dotNL,k)*helper_G_smith(dotNV,k);
}

float F_shlick(float F0, float dotVH)
{
	return F0+(1-F0)*pow((1-dotVH),5);
}


float textureShadow3x3(sampler2DArrayShadow t, vec4 c, vec2 s);

void main()
{  
	float depth = texture(texture3, coord).r;
	outColor = vec4(0,0,0,0);
	
	if(depth == 1)
		return;

	vec4 normal_frag = texture(texture1, coord);
	vec4 material = texture(texture2, coord); // roughness, metal like, specular, emissive
	vec3 color = texture(texture0, coord).xyz;
	vec3 normal = normalize(normal_frag.xyz*2-1);
	vec3 frag_pos = computeFragPos(depth, coord);
	
	vec3 V = normalize(vec3(cameraPos) - frag_pos);
	float dotNV = clamp(dot(normal, V), 0.0001, 0.9999);
	
	float metalic = material.y;
	float roughness = material.x;
	
	outColor += vec4(color * material.w * 5,0);
	
	for(int i=0 ; i<nbLights ; ++i)
	{
		vec3 L = -normalize(lightDirection[i].xyz);
		vec3 H = normalize(L + V);
		
		float dotNL = dot(normal, L);
		float dotNH = dot(normal, H);
		float dotVH = dot(V,H);
		
		if(dotNL <= 0 || dotNV <= 0) continue;
		
		dotNL = clamp(dotNL, 0.0001,0.9999);
		dotNH = clamp(dotNH, 0.0001,0.9999);
		dotVH = clamp(dotVH, 0.0001,0.9999);
		
		float G_CT = G_smith(dotNV, dotNL, roughness);
		float F_CT = F_shlick(mix(0.04, 0.9, metalic), dotVH);
		float D_CT = D_GGX(roughness, dotNH);
		
		float specular = D_CT*G_CT*F_CT / (4*dotNV*dotNL);
		
		vec3 diffuseColor = color - color * metalic;
		vec3 specularColor = mix(vec3(material.z), color, metalic);
		
		float shadowCoef = 1;
		
		//for(float s=0 ; s<lightDirection[i].w ; s+=1)
		if(lightDirection[i].w > 0)
		{
			ivec3 tSize = textureSize(textures[i],0);
			for(int layer=0 ; layer<min(MAX_SHADOW_LVL, tSize.z) ; ++layer)
			{
				vec4 s_coord = (lightMatrix[layer+i*MAX_SHADOW_LVL] * vec4(frag_pos,1));
				s_coord /= s_coord.w;
				
				if(s_coord.x >= 0 && s_coord.x <= 1 && s_coord.y >= 0 && s_coord.y <= 1 && s_coord.z >= 0 && s_coord.z <= 1)
				{
					//shadowCoef = texture(textures[i], vec4(s_coord.xy, float(layer), s_coord.z));
					shadowCoef = textureShadow3x3(textures[i], vec4(s_coord.xy, float(layer), s_coord.z), vec2(tSize.x, tSize.y));
					break;
				}
			}
		}
		
		outColor.xyz += shadowCoef * dotNL * lightColor[i].xyz * (diffuseColor + specularColor*specular);
	}
}

const float BIAS[] = {0.015 / 1000, 0.04 / 1000, 0.16 / 1000, 0.5 / 1000};
//const float BIAS[4] = {0.001, 0.004, 0.01, 0.03};

float textureShadow3x3(sampler2DArrayShadow tex, vec4 coord, vec2 sizeTex)
{
	coord.w -= BIAS[int(coord.z+0.5)];
	float res = texture(tex, coord);
	const float o_clamp = 1.5f/sizeTex.x;
	const float o = 1.5f/sizeTex.y;
	
	vec2 dx = vec2(clamp(coord.x+o,o_clamp,1.f-o_clamp), clamp(coord.x-o,o_clamp,1.f-o_clamp));
	vec2 dy = vec2(clamp(coord.y+o,o_clamp,1.f-o_clamp), clamp(coord.y-o,o_clamp,1.f-o_clamp));
	
	res += texture(tex, vec4(dx.x, coord.yzw))*0.5;
	res += texture(tex, vec4(dx.y, coord.yzw))*0.5;
	res += texture(tex, vec4(coord.x, dy.x, coord.zw))*0.5;
	res += texture(tex, vec4(coord.x, dy.y, coord.zw))*0.5;
	
	res += texture(tex, vec4(dx.x, dy.x, coord.zw))*0.25;
	res += texture(tex, vec4(dx.y, dy.x, coord.zw))*0.25;
	res += texture(tex, vec4(dx.x, dy.y, coord.zw))*0.25;
	res += texture(tex, vec4(dx.y, dy.y, coord.zw))*0.25;
	
	return res*0.25; 
}
