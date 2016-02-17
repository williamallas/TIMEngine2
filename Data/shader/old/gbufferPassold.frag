#version 330  
  
 smooth in vec3 in_normal;  
#ifdef USE_TEXTURE  
 in vec2 texCoord0;  
 uniform sampler2D texture0;  
#endif  
 
#ifdef NORMAL_MAPPING  
 smooth in vec3 in_tangent;  
 uniform sampler2D texture1;  
 #ifndef USE_TEXTURE  
    in vec2 texCoord0;  
 #endif  
#endif  
  
#ifdef DIRECTIONAL_LIGHT_TEST  
 in vec3 vertexWorld;  
 uniform vec3 cameraWorld;  
#endif  
  
 uniform vec4 color;  
 uniform vec4 material;  
 uniform vec4 material2;  
 layout(location=0) out vec4 outColor0;  
#ifdef DEFERRED  
 layout(location=1) out vec4 outColor1;  
 layout(location=2) out vec4 outColor2;  
#endif  
    //#ifdef BONES_MATRIX  
  
 void main()  
 {  
    outColor0=color;  
#ifdef USE_TEXTURE  
    outColor0 *= texture(texture0, texCoord0);  
#endif  
 vec3 norm = in_normal;  
#ifdef NORMAL_MAPPING  
 norm = normalize(norm);  
 vec3 t=normalize(in_tangent);  
 mat3 tbn = mat3(t, cross(t,norm), norm);  
 norm = tbn*(texture(texture1, texCoord0).xyz*2-1);  
#endif  
  
#ifdef DIRECTIONAL_LIGHT_TEST  
    vec4 col=outColor0;  
    vec3 dLightTestColor=vec3(0.2,1,1);  
    vec3 dLightTestDir=normalize(vec3(0.5,-0.5,-0.7));  
    outColor0 = col*dLightTestColor.r*material.r; // ambient  
    outColor0 += col*max(0,dot(norm, -dLightTestDir))*dLightTestColor.g*material.g; // diffuse  
     
	vec3 vDir = normalize(cameraWorld - vertexWorld);  
	outColor0 += col*material.z*pow(max(dot(reflect(dLightTestDir, norm), vDir),0), material2.x);  
    outColor0 += col*material.w;  
    outColor0.a=col.a;  
#endif  
#ifdef DRAW_NORMAL  
 outColor0.xyz=normalize(norm*0.5+0.5);  
#endif  
#ifdef DEFERRED  
    int pack_exp_ref = int(min(256.f,material2.x));  
    pack_exp_ref += int(256*material2.y)*256;  
    #ifndef ZERO_NORMAL  
     outColor1 = vec4(norm*0.5+0.5, clamp(float(pack_exp_ref)/(256*256), 0, 1));  
     outColor2 = material;  
    #else  
     outColor1 = vec4(0,0,0, clamp(float(pack_exp_ref)/(256*256), 0, 1));  
     outColor2 = material;  
    #endif  
#endif  
 }