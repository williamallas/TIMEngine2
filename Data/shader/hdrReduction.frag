#version 330

uniform sampler2D texture0;

#ifdef STEP_1
uniform sampler2D texture1; // previous 1x1 texture
#endif

#ifdef LAST_STEP
const ivec2 textureRes = ivec2(2,2);
uniform float time; 
uniform vec2 minMaxKey;
#else
uniform ivec2 textureRes;
#endif


smooth in vec2 texc;

layout(location=0) out vec4 outColor0;  

float lumi(vec3 col)
{
	return  0.299*col.r + 0.587*col.g + 0.114*col.b;
}

void main()
{

#ifdef STEP_1
	const float DELTA = 0.0001f;
	float l = lumi(texture(texture0, texc).xyz);
	
	vec4 prevHdr = texture(texture1, vec2(0.5,0.5));
	
	outColor0 = vec4(l, l, log(DELTA+l), prevHdr.w);
	return;
#endif

	vec2 offset = vec2(0.5f) / vec2(textureRes.x, textureRes.y);
	
	vec4 sample1 = texture(texture0, texc + offset*vec2(-1,-1));
	vec4 sample2 = texture(texture0, texc + offset*vec2(1,-1));
	vec4 sample3 = texture(texture0, texc + offset*vec2(-1,1));
	vec4 sample4 = texture(texture0, texc + offset*vec2(1,1));
	
	float lumiMax = max(sample1.x, sample2.x);
	lumiMax = max(lumiMax, sample3.x);
	lumiMax = max(lumiMax, sample4.x);
	
	float logAvgLumi = (sample1.z + sample2.z + sample3.z + sample4.z) * 0.25;
	
	float avgLumi = (sample1.y + sample2.y + sample3.y + sample4.y) * 0.25;
	
	#ifdef LAST_STEP
	
	int diff = logAvgLumi - sample1.w < 0 ? -1:1;
	float newLogAvg = sample1.w + diff*min(time, diff*(logAvgLumi - sample1.w));
	
	newLogAvg = clamp(newLogAvg, log(minMaxKey.x),log(minMaxKey.y));
	
	outColor0 = vec4(lumiMax, avgLumi, exp(newLogAvg), newLogAvg);
	
	#else
	outColor0 = vec4(lumiMax, avgLumi, logAvgLumi, sample1.w);
	#endif
}