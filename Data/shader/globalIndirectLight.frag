#version 430
#define NB_MIPMAP 7
 
precision highp float;
const float PI = 3.14159265359;

uniform sampler2D textures[4];
uniform samplerCube texture4; // skybox
uniform samplerCube texture5; // blurred skybox
uniform sampler2D texture6; // brdf

in vec2 coord;
 
layout(location=0) out vec4 outColor; 

layout(std140, binding = 0) uniform DrawParameter
{
	mat4 view, proj;
	mat4 projView, invView, invProj, invProjView, worldOriginInvProjView;
	vec4 cameraPos, cameraUp, cameraDir, worldOrigin;
	vec4 time;
}; 

vec3 computeFragPos(float depth, vec2 texCoord)
{
	vec4 position = invProjView * vec4((texCoord.st-vec2(0.5))*2.0f, (depth-0.5)*2.0f, 1);
	return position.xyz / position.w;
}

vec3 approximateSpecular(vec3 specColor, float roughness, vec3 r, float dotNV)
{
	const float rough[NB_MIPMAP] = {0, 0.05, 0.13, 0.25, 0.45, 0.66, 1};
	int index=NB_MIPMAP-2;
	for(int i=0 ; i<NB_MIPMAP-1 ; ++i)
	{
		if(rough[i] >= roughness)
		{
			index=i;
			break;
		}
	}
	
	float coef = (roughness - rough[index]) / (rough[index+1] - rough[index]);
	
	vec2 a_b = texture(texture6, vec2(dotNV, roughness)).xy;
	
	return textureLod(texture5, r, float(index)+coef).rgb * (specColor*a_b.x+vec3(a_b.y));
}

void main()
{  
	outColor = vec4(0,0,0,0);
	
	float depth = texture(textures[3], coord).r;
	vec4 normal_frag = texture(textures[1], coord);
	vec4 material = texture(textures[2], coord); // roughness, metal like, specular 
	vec4 color = texture(textures[0], coord);
	vec3 normal = normalize(normal_frag.xyz*2-1);
	vec3 frag_pos = computeFragPos(depth, coord);
	
	float roughness = material.x;
	float metallic = material.y;
	
	vec3 viewDir = normalize(frag_pos - vec3(cameraPos));

	if(depth == 1)
	{
		outColor = texture(texture4, viewDir);
		//outColor = textureLod(texture5, viewDir, NB_MIPMAP-2);
		outColor.a=0;
	}
	else
	{
		vec4 diffuseTerm = color * (1-material.y) * textureLod(texture5, normal,NB_MIPMAP-1);
		
		vec3 specularColor = mix(vec3(material.z), vec3(color), metallic);
		vec3 specTerm = approximateSpecular(specularColor ,roughness, reflect(viewDir, normal), max(dot(normal, -viewDir), 0.01));
		
		outColor.rgb = specTerm + vec3(diffuseTerm);
	}
	
	
}
