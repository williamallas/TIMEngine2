#version 330

 smooth in vec3 in_normal;
#ifdef USE_TEXTURE
 in vec2 texCoord0;
 uniform sampler2D texture2;
#endif

#ifdef DIRECTIONAL_LIGHT_TEST
 in vec3 vertexWorld;
 uniform vec3 cameraWorld;
#endif

 uniform vec4 color;
 uniform vec4 material;
 uniform float exponent;
 layout(location=0) out vec4 outColor0;
#ifdef DEFERRED
 layout(location=1) out vec4 outColor1;
 layout(location=2) out vec4 outColor2;
#endif

 void main()
 {
	outColor0=color;
	float alpha=1;
#ifdef USE_TEXTURE
	vec4 tcol =  texture(texture2, texCoord0);
	if(tcol.a < 0.5) discard; 
    outColor0 *= tcol;
	alpha *= tcol.a;
#endif
	
	alpha *= color.a;
#ifdef DIRECTIONAL_LIGHT_TEST
    vec4 col=outColor0;
    vec3 dLightTestColor=vec3(0.5,1,1);
    vec3 dLightTestDir=normalize(vec3(-0.5,0.5,-0.7));
    outColor0 = col*dLightTestColor.r*material.r; // ambient
    outColor0 += col*max(0,dot(in_normal, -dLightTestDir))*dLightTestColor.g*material.g; // diffuse
   
	vec3 vDir = normalize(cameraWorld - vertexWorld);
	outColor0 += col*material.z*pow(max(dot(reflect(dLightTestDir, in_normal), vDir),0), exponent);
    outColor0 += col*material.w;
#endif
	outColor0.a = alpha;
	
#ifdef DRAW_NORMAL
	outColor0.xyz=in_normal*0.5+0.5;
#endif
#ifdef DEFERRED
	outColor1 = vec4(in_normal*0.5+0.5, clamp(exponent/1024, 0, 1));
	outColor2 = material;
#endif
 }