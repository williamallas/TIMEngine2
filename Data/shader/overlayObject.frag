#version 430

smooth in vec3 v_normal;
  
layout(location=0) out vec4 outColor; 
layout(location=1) out vec4 outNormal;
layout(location=2) out vec4 outMaterial;
  
void main()  
{  	
	vec3 n = normalize(v_normal);
	
	outColor = vec4(1,1,0,1);
	outNormal = vec4(0.5,0.5,0.5,1);
	outMaterial = vec4(0,0,0,0.2);
}