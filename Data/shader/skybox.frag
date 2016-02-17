#version 330 
 
uniform vec4 color; 
uniform vec4 material; 
uniform vec4 material2; 
layout(location=0) out vec4 outColor0; 
#ifdef DEFERRED 
 layout(location=1) out vec4 outColor1; 
 layout(location=2) out vec4 outColor2; 
#endif 
 
uniform samplerCube texture0;

smooth in vec3 vDir;
smooth in vec2 texCoord;
 
 void main() 
 { 
    outColor0=color; 
    outColor0 *= texture(texture0, vDir); 
	outColor0.a=1; 

#ifdef DEFERRED 
    int pack_exp_ref = int(min(256.f,material2.x)); 
    pack_exp_ref += int(256*material2.y)*256; 

     outColor1 = vec4(0,0,0, clamp(float(pack_exp_ref)/(256*256), 0, 1)); 
     outColor2 = material; 
#endif 
 }