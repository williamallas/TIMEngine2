#version 430
 
precision highp float;

uniform sampler2D textures[4];

#define MAX_SHADOW_LIGHT 3
#define MAX_SHADOW_LIGHT_LEVEL 4
#define SHADOW_COEF 0.1
uniform sampler2DArrayShadow shadowMaps[MAX_SHADOW_LIGHT];
uniform mat4 shadowLightMatrix[MAX_SHADOW_LIGHT*MAX_SHADOW_LIGHT_LEVEL];
 
layout(location=0) out vec4 outColor0; 
in vec2 coord;

#define MAX_LIGHT 5
uniform vec4 lightDirection[MAX_LIGHT];
uniform vec4 lightDiffuse[MAX_LIGHT];
uniform vec4 lightSpecular[MAX_LIGHT];
uniform int nbLight;
uniform vec4 globalAmbient;

uniform vec3 worldOrigin;
uniform mat4 worldOriginInvProjView;
uniform vec3 cameraWorld;

vec3 computeFragPos(float depth, vec2 texCoord)
{
	vec4 position = worldOriginInvProjView * vec4((texCoord.st-vec2(0.5))*2.0f, (depth-0.5)*2.0f, 1);
	return position.xyz / position.w;
}

const float BIAS[] = {0.001, 0.0015, 0.0025, 0.004};
float textureShadow3x3(sampler2DArrayShadow tex, vec4 coord, vec2 sizeTex);

void main()
{  
	float depth = texture(textures[3], coord).r;
	outColor0 = vec4(0,0,0,0);
	if(depth == 1)
		return;
	
	vec4 normal_frag = texture(textures[1], coord);
	vec4 material = texture(textures[2], coord);
	vec4 color = texture(textures[0], coord);
	vec3 normal = normalize(normal_frag.xyz*2-1);
	vec3 frag_pos = computeFragPos(depth, coord);
	
	int indexShadow=0;
	
	for(int i=0 ; i<nbLight ; ++i)
	{
		float coefShadow=1;
		const float O=0.01;
		if(lightDirection[i].w > 0)
		{
			ivec3 tSize = textureSize(shadowMaps[indexShadow],0);
			for(int j=0 ; j<min(tSize.z,4) ; ++j)
			{
				vec3 coord = (shadowLightMatrix[indexShadow*4+j] * vec4(frag_pos,1)).xyz;
				if(coord.y <= 1-O && coord.y >= 0+O && coord.x <= 1-O && coord.x >= 0+O && coord.z <= 1-O && coord.z >= 0+O)
				{
					//coefShadow = texture(shadowMaps[indexShadow], vec4(coord.xy, 0, coord.z-BIAS[i]));
					coefShadow = textureShadow3x3(shadowMaps[indexShadow], vec4(coord.xy, j, coord.z-BIAS[j]), vec2(tSize.x,tSize.y));
					break;
				}
			}
			
			indexShadow++;
			coefShadow = coefShadow * (1.f-SHADOW_COEF) + SHADOW_COEF;
		}
		
		// Diffuse
		vec3 lightDir = -normalize(lightDirection[i].xyz);
		float lightCoef = clamp(dot(lightDir, normal),0,1);
		outColor0 += (coefShadow * lightCoef * color * material.y * lightDiffuse[i]);
	
		// Specular
		if(lightSpecular[i].xyz != vec3(0,0,0))
		{
			vec3 R = reflect(-lightDir, normal);
			int packed_exp_ref = int(normal_frag.w*256*256);
			float shininess = float(packed_exp_ref & 0xff);
			vec3 vertexDir = normalize(cameraWorld-worldOrigin-frag_pos);

			lightCoef = pow(max(dot(R, vertexDir),0), shininess);
			outColor0 += (coefShadow * lightCoef * color * material.z * lightSpecular[i]);
		}
	}
	
	outColor0.a=0;
}

float textureShadow3x3(sampler2DArrayShadow tex, vec4 coord, vec2 sizeTex)
{
	coord.w -= BIAS[int(coord.z+0.5)];
	float res = texture(tex, coord);
	const float o_clamp = 1.f/sizeTex.x;
	const float o = 1.f/sizeTex.y;
	
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