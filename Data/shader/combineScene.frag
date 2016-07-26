#version 430

#ifdef STEREO_DISPLAY
uniform sampler2D textures[2];
#else
uniform sampler2D textures[6];
uniform int nbScene;
#endif

smooth in vec2 texc;

layout(location=0) out vec4 outColor0;

void main()
{
#ifdef STEREO_DISPLAY
	if(texc.x < 0.499) outColor0 = texture(textures[1], vec2(texc.x*2,texc.y));
	else if(texc.x > 0.501) outColor0 = texture(textures[0], vec2((texc.x-0.5)*2,texc.y));
	else outColor0 = vec4(1,0,0,1);
	
	//outColor0 = vec4(texc,0,1);
#else
	vec4 mat = texture(textures[0], texc); 
	outColor0 = texture(textures[1], texc);
	
	if(mat.x==1&&mat.y==1) 
	{
		if(mat.z < 0.1) outColor0 = texture(textures[2], texc);
		else if(mat.z < 0.1+0.2) outColor0 = texture(textures[3], texc);
		else if(mat.z < 0.1+0.4) outColor0 = texture(textures[4], texc);
		else if(mat.z < 0.1+0.6) outColor0 = texture(textures[5], texc);
	}
#endif

	outColor0.a = 1;
}