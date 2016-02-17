#version 330  
  
 smooth in vec3 in_normal;  
 uniform sampler2D texture0;     
 smooth in vec3 in_tangent;  
 uniform sampler2D texture1;   
 in vec2 texCoord0;  
 uniform float time;
 
 smooth in vec3 vDir;
 uniform vec3 cameraWorld;
  
 uniform vec4 color;  
 uniform vec4 material;  
 uniform vec4 material2;  
 layout(location=0) out vec4 outColor0;  
#ifdef DEFERRED  
 layout(location=1) out vec4 outColor1;  
 layout(location=2) out vec4 outColor2;  
#endif  
  
 void main()  
 { 
	float coefTime=0.02;
	vec3 n1 = texture(texture1, texCoord0+time*vec2(0.5,0.4)*coefTime).xyz*2-1;
	vec3 n2 = texture(texture1, texCoord0+time*vec2(0.4,-0.5)*coefTime).xyz*2-1;
	
	vec3 norm = in_normal;  
	norm = normalize(norm);  
	vec3 t=normalize(in_tangent);  
	mat3 tbn = mat3(t, cross(t,norm), norm);  
	norm = tbn*normalize(n1+n2);   
	
	outColor0=color;  
    outColor0 *= texture(texture0, texCoord0);
	
	#ifdef FRENSEL
	vec3 viewDir = normalize(cameraWorld-vDir);
	float fresnel = min(0.8,max(0.2,dot(norm, viewDir)));
	fresnel = pow(fresnel, 1.5);
	#endif
	
   
#ifdef DEFERRED  
    int pack_exp_ref = int(min(256.f,material2.x)); 
	
	#ifdef FRENSEL
    pack_exp_ref += int(256*/*material2.y*/(1.f-fresnel))*256;  
    outColor2 = material*fresnel; 
	#else
	pack_exp_ref += int(256*material2.y)*256;  
    outColor2 = material; 
	#endif
	 
    outColor1 = vec4(norm*0.5+0.5, clamp(float(pack_exp_ref)/(256*256), 0, 1));  
     
#endif 
 } 