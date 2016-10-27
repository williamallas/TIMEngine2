#version 430
#define NB_MIPMAP 7
 
precision highp float;
const float PI = 3.14159265359;

//uniform sampler2D textures[7];
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

uniform samplerCube texture4; // skybox
uniform samplerCube texture5; // blurred skybox
uniform sampler2D texture6; // brdf
uniform sampler2D texture7; // reflexion

uniform sampler2DArrayShadow texture8;

uniform int enableGI;
uniform vec4 globalAmbient;
uniform int localReflexion;

#define MAX_LIGHTS 8
#define MAX_SHADOW_LVL 4
uniform int nbLights;
uniform vec4 lightDirection[MAX_LIGHTS];
uniform vec4 lightColor[MAX_LIGHTS];
uniform mat4 lightMatrix[MAX_SHADOW_LVL];

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

vec3 unproj(vec2 texCoord, float depth)
{
	vec4 position = invProj	* vec4((texCoord.st-vec2(0.5))*2.0f, (depth-0.5)*2.0f, 1);
	return position.xyz / position.w;
}

vec3 unprojWorld(vec2 texCoord, float depth)
{
	vec4 position = invProjView	* vec4((texCoord.st-vec2(0.5))*2.0f, (depth-0.5)*2.0f, 1);
	return position.xyz / position.w;
}

vec3 project(vec3 pos)
{
	vec4 position = proj * vec4(pos, 1);
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

vec4 raytraceCamSpace(vec3 rayPos, vec3 rayDir, sampler2D ZBuffer, vec3 n, vec4 backColor);

vec3 approximateSpecular(vec3 specColor, float roughness, vec3 r, float dotNV);

void main()
{  
	outColor = vec4(0,0,0,0);
	
	float depth = texture(texture3, coord).r;
	vec4 normal_frag = texture(texture1, coord);
	vec4 material = texture(texture2, coord); // roughness, metallic, specular, emissive 
	vec4 color = texture(texture0, coord);
	vec3 normal = normalize(normal_frag.xyz*2-1);
	vec3 frag_pos = computeFragPos(depth, coord);
	
	float roughness = material.x;
	float metallic = material.y;
	
	vec3 viewDir = normalize(frag_pos - vec3(cameraPos));
	
	if(material.x==1 && material.y==1)
	{
		return;
	}
	if(depth == 1)
	{
		outColor = texture(texture4, viewDir);
		outColor.a=0;
		return;
	}
	else if(roughness < 0.01 && localReflexion == 1) // raytrace camspace
	{
		vec3 world = unproj(coord, depth);
		vec3 I = normalize(world);
		vec3 n =  mat3(view) * normalize(normal_frag.xyz*2-1);
		vec3 reflected = normalize(reflect(I, n));
		
		vec4 specColor = mix(vec4(material.z), color, material.y);
		outColor = specColor*raytraceCamSpace(world, reflected, texture3, n, texture(texture4, reflect(viewDir, normal)));
	}
	else if(enableGI == 1)
	{
		vec4 diffuseTerm = color * (1-material.y) * textureLod(texture5, normal,NB_MIPMAP-1);
		
		vec3 specularColor = mix(vec3(material.z), vec3(color), metallic);
		vec3 specTerm = approximateSpecular(specularColor ,roughness, reflect(viewDir, normal), max(dot(normal, -viewDir), 0.01));
		
		outColor.rgb = specTerm + vec3(diffuseTerm);
	}
/* 	else
	{
		vec4 diffuseTerm = color * (1-material.y) * globalAmbient;
		vec2 a_b = texture(texture6, vec2(max(dot(normal, -viewDir), 0.01), roughness)).xy;
		
		vec3 specularColor = mix(vec3(material.z), vec3(color), metallic);
		vec3 specTerm = globalAmbient.rgb * (specularColor*a_b.x + vec3(a_b.y));
		
		outColor.rgb = specTerm + vec3(diffuseTerm);
	} */
	
	outColor += vec4(color * material.w * 5);
	float dotNV = clamp(dot(normal, -viewDir), 0.0001, 0.9999);
		
	for(int i=0 ; i<nbLights ; ++i)
	{
		vec3 L = -normalize(lightDirection[i].xyz);
		vec3 H = normalize(L - viewDir);
		
		float dotNL = dot(normal, L);
		float dotNH = dot(normal, H);
		float dotVH = dot(-viewDir,H);
		
		if(dotNL <= 0 || dotNV <= 0) continue;
		
		dotNL = clamp(dotNL, 0.0001,0.9999);
		dotNH = clamp(dotNH, 0.0001,0.9999);
		dotVH = clamp(dotVH, 0.0001,0.9999);
		
		float G_CT = G_smith(dotNV, dotNL, roughness);
		float F_CT = F_shlick(mix(0.04, 0.9, metallic), dotVH);
		float D_CT = D_GGX(roughness, dotNH);
		
		float specular = D_CT*G_CT*F_CT / (4*dotNV*dotNL);
		
		vec3 diffuseColor = color.xyz - color.xyz * metallic;
		vec3 specularColor = mix(vec3(material.z), color.xyz, metallic);
		
		float shadowCoef = 1;
		
		//for(float s=0 ; s<lightDirection[i].w ; s+=1)
		if(lightDirection[i].w > 0)
		{
			ivec3 tSize = textureSize(texture8,0);
			for(int layer=0 ; layer<min(MAX_SHADOW_LVL, tSize.z) ; ++layer)
			{
				vec4 s_coord = (lightMatrix[layer] * vec4(frag_pos,1));
				s_coord /= s_coord.w;
				
				if(s_coord.x >= 0 && s_coord.x <= 1 && s_coord.y >= 0 && s_coord.y <= 1 && s_coord.z >= 0 && s_coord.z <= 1)
				{
					//shadowCoef = texture(textures[i], vec4(s_coord.xy, float(layer), s_coord.z));
					shadowCoef = textureShadow3x3(texture8, vec4(s_coord.xy, float(layer), s_coord.z), vec2(tSize.x, tSize.y));
					break;
				}
			}
		}
		
		outColor.xyz += shadowCoef * dotNL * lightColor[i].xyz * (diffuseColor + specularColor*specular);
	}
	
	outColor.a = 0;
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

vec4 binarySearch(vec3 noP, vec3 yesP, sampler2D ZBuffer, out vec3 hitP)
{
	vec3 m, result=yesP;
	
	for(int i=0 ; i<10 ; ++i)
	{
		m = (noP+yesP)*0.5;
		vec3 proj_P = project(m)*0.5+0.5;
		float d = texture(ZBuffer, proj_P.xy).r;
		
		if(proj_P.z > d) yesP = m;
		else noP = m;
		
		result = yesP;
	}

	result = project(yesP)*0.5+0.5;
	hitP = vec3(yesP.xy, unproj(result.xy, texture(ZBuffer, result.xy).r).z);
	return texture(texture0, result.xy);
}

vec4 computePlan(vec3 p, vec3 n)
{
	n = normalize(n);
	return vec4(n, -dot(n,p));
}

float dist(vec4 plan, vec3 p)
{
	return dot(plan.xyz, p) + plan.w;
} 

vec4 raytraceCamSpace(vec3 rayPos, vec3 rayDir, sampler2D ZBuffer, vec3 n, vec4 backColor)
{
	const float maxDist = 8;
	const float maxStep = 15.0;
	
	//float d_e = dist(computePlan(rayPos, n), rayPos+rayDir);
	
	vec3 rayStep = rayDir;// * (maxDist / maxStep);
	rayStep /= length(rayStep.xy);
	rayStep *= (maxDist/maxStep);
	vec3 P=rayPos;
	float step=0;
	vec3 prevP=vec3(rayPos);
	
	for(; step<maxStep ; step+=1, P+=rayStep)
	{
		vec3 curP = P+rayStep*0.5;
		vec3 proj_P = project(curP);
		if(!all(lessThanEqual(abs(proj_P), vec3(1.f))))
			return backColor;
		
		proj_P = proj_P*0.5+0.5;
		float d = texture(ZBuffer, proj_P.xy).r;
		
		if(proj_P.z > d) // LE NEAR = 0 le FAR = -inf
		{
			vec3 hitP;
			vec4 res = binarySearch(prevP, curP, ZBuffer, hitP);
			
			vec3 hitVec = normalize(hitP - rayPos);
			if(dot(rayDir, hitVec) > 1.f-0.001)
			{
				float c = 1.f-clamp((dist(computePlan(rayPos, n), hitP)-(maxDist*0.4)) / (maxDist*0.4),0,1);
				return res*c + backColor*(1.f-c);
			}
		}
		else prevP = curP;
		
		rayStep *= 1;
	}
	
	return backColor;
}

const float BIAS[] = {0.015 / 500, 0.04 / 500, 0.08 / 500, 0.2 / 500};

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

