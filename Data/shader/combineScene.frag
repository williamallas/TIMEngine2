#version 430

uniform sampler2D textures[6];
uniform int nbScene;

smooth in vec2 texc;

layout(location=0) out vec4 outColor0;

void main()
{
	vec4 mat = texture(textures[0], texc); 
	outColor0 = texture(textures[1], texc);
	
	if(mat.x==1&&mat.y==1) 
	{
		if(mat.z < 0.1) outColor0 = texture(textures[2], texc);
		else if(mat.z < 0.1+0.2) outColor0 = texture(textures[3], texc);
		else if(mat.z < 0.1+0.4) outColor0 = texture(textures[4], texc);
		else if(mat.z < 0.1+0.6) outColor0 = texture(textures[5], texc);
	}
	
	//else if(mat.x>0.9&&mat.y>0.9) outColor0 = texture(textures[3], texc);
	//if(texc.x > 0.5) outColor0 = texture(textures[2], texc);
	

	outColor0.a = 1;
}