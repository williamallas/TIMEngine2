#version 430
 
precision highp float;
const float PI = 3.14159265359;

uniform samplerCube texture0;
uniform float inRoughness;

in vec2 coord;
 
layout(location=0) out vec4 outColorX; 
layout(location=1) out vec4 outColorNX; 
layout(location=2) out vec4 outColorY; 
layout(location=3) out vec4 outColorNY; 
layout(location=4) out vec4 outColorZ; 
layout(location=5) out vec4 outColorNZ; 

vec4 sampleHemisphere(vec3 normal, vec3 up)
{
	vec3 right = normalize(cross(up,normal));
	up = cross(right,normal);
	vec4 color=vec4(0);
	
	float div=0;
	for(float phi = 0; phi < 2*PI; phi += 2*PI/32)
        for(float theta = 0; theta < 0.5*PI; theta += 0.5*PI/16)
	{
		div += 1;
		vec3 temp = cos(phi) * right + sin(phi) * up;
        vec3 sampleVector = cos(theta) * normal + sin(theta) * temp;
        color += texture(texture0, sampleVector) * cos(theta) * sin(theta);
	}
	return color / div;
}

vec3 importanceSampleGGX(vec2 Xi, float roughness, vec3 N)
{
	float a = roughness * roughness;
	float phi = 2 * PI * Xi.x;
	float cosTheta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y));
	float sinTheta = sqrt(1 - cosTheta * cosTheta);
	vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
	
	vec3 upVector = vec3(0,0,1);
	vec3 tangentX = normalize(cross(upVector, N));
	vec3 tangentY = cross(N, tangentX);

	return tangentX * H.x + tangentY * H.y + N * H.z;
}

float radicalInverse_VdC(uint bits) {
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
 }
 
 vec2 hammersley2d(uint i, uint N) {
     return vec2(float(i)/float(N), radicalInverse_VdC(i));
 }

vec4 filterEnvMap(float roughness, vec3 R)
{
	vec3 N = R;
	vec3 V = R;
	float totalWeight=0;
	vec4 prefilteredColor = vec4(0);
	const uint NumSamples = 1<<10;
	
	for( uint i = 0; i < NumSamples; i++ )
	{
		vec2 Xi = hammersley2d( i, NumSamples );
		vec3 H = importanceSampleGGX(Xi, roughness, N);
		vec3 L = reflect(-V,H);
		float NoL = max(dot(N,L), 0);
		if(NoL > 0)
		{
			prefilteredColor += texture(texture0, L) * NoL;
			totalWeight += NoL;
		}
	}
	return prefilteredColor / totalWeight;
}

void main()
{  
	vec3 norm_x = vec3(1, -coord.y, -coord.x);
	vec3 norm_nx= vec3(-1, -coord.y, coord.x);
	vec3 norm_y = vec3(coord.x, 1, coord.y);
	vec3 norm_ny= vec3(coord.x, -1, -coord.y);
	vec3 norm_z = vec3(coord.x, -coord.y, 1);
	vec3 norm_nz= vec3(-coord.x, -coord.y, -1);
	
	if(inRoughness < 1)
	{
		outColorX =  filterEnvMap(inRoughness,norm_x);
		outColorNX =  filterEnvMap(inRoughness,norm_nx);
		outColorY =  filterEnvMap(inRoughness,norm_y);
		outColorNY =  filterEnvMap(inRoughness,norm_ny);
		outColorZ =  filterEnvMap(inRoughness,norm_z);
		outColorNZ =  filterEnvMap(inRoughness,norm_nz);
	}
	else
	{
		outColorX =  sampleHemisphere(norm_x, vec3(0,0,1));
		outColorNX =  sampleHemisphere(norm_nx, vec3(0,0,1));
		outColorY =  sampleHemisphere(norm_y, vec3(0,0,1));
		outColorNY =  sampleHemisphere(norm_ny, vec3(0,0,1));
		outColorZ =  sampleHemisphere(norm_z, vec3(1,0,0));
		outColorNZ =  sampleHemisphere(norm_nz, vec3(1,0,0));
	}
	
}
