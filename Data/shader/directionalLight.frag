#version 430
 
precision highp float;
const float PI = 3.14159265359;

uniform sampler2D textures[4];

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
uniform int nbLights;
uniform vec4 lightDirection[MAX_LIGHTS];
uniform vec4 lightColor[MAX_LIGHTS];

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

void main()
{  
	float depth = texture(textures[3], coord).r;
	outColor = vec4(0,0,0,0);
	
	if(depth == 1)
		return;
	
	vec4 normal_frag = texture(textures[1], coord);
	vec4 material = texture(textures[2], coord); // roughness, metal like, specular 
	vec3 color = texture(textures[0], coord).xyz;
	vec3 normal = normalize(normal_frag.xyz*2-1);
	vec3 frag_pos = computeFragPos(depth, coord);
	
	vec3 V = normalize(vec3(cameraPos) - frag_pos);
	float dotNV = max(dot(normal, V),0.0001);
	
	float metalic = material.y;
	float roughness = material.x + 0.02;
	
	for(int i=0 ; i<nbLights ; ++i)
	{
		vec3 L = -normalize(lightDirection[i].xyz);
		vec3 H = normalize(L + V);
		
		float dotNL = max(dot(normal, L),0);
		float dotNH = max(dot(normal, H),0);
		float dotVH = max(dot(V,H), 0);
		
		if(dotNL <= 0 || dotNV <= 0) continue;
		
		float G_CT = G_smith(dotNV, dotNL, roughness);
		float F_CT = F_shlick(mix(0.04, 0.9, metalic), dotVH);
		float D_CT = D_GGX(roughness, dotNH);
		
		float specular = D_CT*G_CT*F_CT / (4*dotNV*dotNL);
		
		vec3 diffuseColor = color - color * metalic;
		vec3 specularColor = mix(vec3(material.z), color, metalic);
		
		outColor.xyz += dotNL * lightColor[i].xyz * (diffuseColor + specularColor*specular);
	}
}
