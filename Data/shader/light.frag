#version 430

#define LIGHT_BIAS 1.f/10.f
 
precision highp float;

flat in vec4 light_head; // (type, radius, x, x)
flat in vec4 light_diffuse;
flat in vec4 light_specular;
flat in vec4 light_attenuation;
flat in vec4 light_spotData;
flat in vec3 light_position;

uniform sampler2D textures[4];

uniform vec3 worldOrigin;
uniform mat4 worldOriginInvProjView;
uniform vec3 cameraWorld;
 
layout(location=0) out vec4 outColor0; 

vec3 computeFragPos(float depth, vec2 texCoord)
{
	vec4 position = worldOriginInvProjView * vec4((texCoord.st-vec2(0.5))*2.0f, (depth-0.5)*2.0f, 1);
	return position.xyz / position.w;
}

void main()
{  
	outColor0 = vec4(0,0,0,0);
	float depth = texelFetch(textures[3], ivec2(gl_FragCoord.xy),0).r;
	if(depth == 1)
		return;
	
	vec4 normal_frag = texelFetch(textures[1], ivec2(gl_FragCoord.xy),0);
	vec4 material = texelFetch(textures[2], ivec2(gl_FragCoord.xy),0);
	vec4 color = texelFetch(textures[0], ivec2(gl_FragCoord.xy),0);
	
	vec3 normal = normalize(normal_frag.xyz*2-1);
	
	vec2 coord = gl_FragCoord.xy / textureSize(textures[0],0).xy;
	vec3 frag_pos = computeFragPos(depth, coord);
	
	vec3 lightDir = light_position - worldOrigin - frag_pos;
	float dist = max(0,length(lightDir) - light_attenuation.w);
	lightDir = normalize(lightDir);
	
	// Diffuse 
	float att = 1.0 / (dist * dist * light_attenuation.z + dist * light_attenuation.y + light_attenuation.x) - LIGHT_BIAS;
	att = max(0, att);

	float lightCoef = clamp(dot(lightDir, normal),0,1);
	
	outColor0 += (lightCoef * color * material.y * light_diffuse * att);
	
	// Specular
	if(light_specular.xyz == vec3(0,0,0))
	{
		outColor0.a=0;
		return;
	}
	
	vec3 R = reflect(-lightDir, normal);
	int packed_exp_ref = int(normal_frag.w*256*256);
	float shininess = float(packed_exp_ref & 0xff);
	vec3 vertexDir = normalize(cameraWorld-worldOrigin-frag_pos);

	lightCoef = pow(max(dot(R, vertexDir),0), shininess);
	outColor0 += ( lightCoef * color * material.z * light_specular * att);
	
	outColor0.a=0;

}