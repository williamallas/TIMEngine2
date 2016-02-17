#version 330

uniform sampler2D texture0;
uniform ivec2 textureRes;

smooth in vec2 texc;

layout(location=0) out vec4 outColor0;  


vec3 blur5_sigma_1_25(vec2 tc)
{
	const float kernel[5] = float[]( 0.09672, 0.240449, 0.325661, 0.240449, 0.09672 );
	
	#ifdef V_PASS
	const vec2 d[5] = vec2[]( vec2(0,-2),vec2(0,-1),vec2(0,0),vec2(0,1),vec2(0,2) );
	float o = 1.f / textureRes.y;
	#endif
	#ifdef H_PASS
	const vec2 d[5] = vec2[]( vec2(-2,0),vec2(-1,0),vec2(0,0),vec2(1,0),vec2(2,0) );
	float o = 1.f / textureRes.x;
	#endif
	
	vec3 res;
	for(int i=0 ; i<5 ; ++i)
		res += kernel[i]*texture(texture0, texc+o*d[i]).xyz;
	return res;
}

vec3 blur5_sigma_2_5(vec2 tc)
{
	const float kernel[5] = float[]( 0.169327, 0.214574, 0.232198, 0.214574, 0.169327 );
	
	#ifdef V_PASS
	const vec2 d[5] = vec2[]( vec2(0,-2),vec2(0,-1),vec2(0,0),vec2(0,1),vec2(0,2) );
	float o = 1.f / textureRes.y;
	#endif
	#ifdef H_PASS
	const vec2 d[5] = vec2[]( vec2(-2,0),vec2(-1,0),vec2(0,0),vec2(1,0),vec2(2,0) );
	float o = 1.f / textureRes.x;
	#endif
	
	vec3 res;
	for(int i=0 ; i<5 ; ++i)
		res += kernel[i]*texture(texture0, texc+o*d[i]).xyz;
	return res;
}

vec3 blur7_sigma_3_5(vec2 tc)
{
	const float kernel[7] = float[]( 0.115528, 0.141488, 0.159786, 0.166396, 0.159786, 0.141488, 0.115528 );
	
	#ifdef V_PASS
	const vec2 d[7] = vec2[]( vec2(0,-3),vec2(0,-2),vec2(0,-1),vec2(0,0),vec2(0,1),vec2(0,2),vec2(0,3) );
	float o = 1.f / textureRes.y;
	#endif
	#ifdef H_PASS
	const vec2 d[7] = vec2[]( vec2(-3,0),vec2(-2,0),vec2(-1,0),vec2(0,0),vec2(1,0),vec2(2,0),vec2(3,0) );
	float o = 1.f / textureRes.x;
	#endif
	
	vec3 res;
	for(int i=0 ; i<7 ; ++i)
		res += kernel[i]*texture(texture0, texc+o*d[i]).xyz;
	return res;
}

vec3 blur7_sigma_2(vec2 tc)
{
	const float kernel[7] = float[]( 0.071303, 0.131514, 0.189879, 0.214607, 0.189879, 0.131514, 0.071303 );
	
	#ifdef V_PASS
	const vec2 d[7] = vec2[]( vec2(0,-3),vec2(0,-2),vec2(0,-1),vec2(0,0),vec2(0,1),vec2(0,2),vec2(0,3) );
	float o = 1.f / textureRes.y;
	#endif
	#ifdef H_PASS
	const vec2 d[7] = vec2[]( vec2(-3,0),vec2(-2,0),vec2(-1,0),vec2(0,0),vec2(1,0),vec2(2,0),vec2(3,0) );
	float o = 1.f / textureRes.x;
	#endif
	
	vec3 res;
	for(int i=0 ; i<7 ; ++i)
		res += kernel[i]*texture(texture0, texc+o*d[i]).xyz;
	return res;
}

void main()
{
	outColor0 = vec4(blur7_sigma_3_5(texc), 1);
}