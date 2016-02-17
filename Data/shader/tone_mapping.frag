#version 330

uniform sampler2D textures[2];

uniform float exposure;
uniform float whiteSaturation;

smooth in vec2 texc;

layout(location=0) out vec4 outColor0;  


float lumi(vec3 col)
{
	return  0.299*col.r + 0.587*col.g + 0.114*col.b;
}

vec3 rgbBrightConversion(vec3 rgb, float bright)
{
	float coef = bright / lumi(rgb);
	return coef * rgb;
}

vec3 tone_map_reinhard(vec3 col, float exp, float key)
{
	float packedLumi = exp * lumi(col) / key;
	packedLumi = packedLumi / (packedLumi + 1);
	return rgbBrightConversion(col, packedLumi);
}

vec3 reinhardWhite(vec3 col, float exp, float key, float minWhite)
{
	float packedLumi = exp * lumi(col) / key;
	
	float whiteFactor = 1 + packedLumi/(minWhite*minWhite);
	
	packedLumi = packedLumi*whiteFactor / (packedLumi + 1);
	return rgbBrightConversion(col, packedLumi);
}

int exceedRange(vec3 color)
{
	if(color.x > 1 || color.y > 1 || color.z > 1)
		return 1;
	else return 0;
}

vec3 gammaCorrection(vec3 col)
{
	return pow(col, vec3(1/2.2f));
}

void main()
{
	vec3 color = texture(textures[0], texc).xyz;
	vec4 hdrData = texture(textures[1], vec2(0.5,0.5));
	
	//color = max_lumi_tone_mapping(color, exposure, hdrData.x);
	color = reinhardWhite(color, exposure, hdrData.z, whiteSaturation);
	
	outColor0 = vec4(color,1);
}