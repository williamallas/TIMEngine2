#version 430
 
precision highp float;

uniform sampler2D textures[4];
 
layout(location=0) out vec4 outColor0; 
in vec2 coord;

uniform vec4 globalAmbient;

void main()
{  
	float depth = texture(textures[3], coord).r;
	outColor0 = vec4(0,0,0,0);
	//return;
	
	vec4 material = texture(textures[2], coord);
	vec4 color = texture(textures[0], coord);

	outColor0 += color * material.w;
	
	if(depth == 1){
		outColor0 += color;
		outColor0.a=1;
		return;
	}

	outColor0 += globalAmbient * color * material.x; 

	outColor0.a=1;
}