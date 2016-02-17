#version 330  
  
uniform vec4 color;  
uniform vec4 material;  
uniform float exponent;  

uniform sampler2D texture1;  
uniform sampler2D texture2;  // mask
uniform vec2 textureScale2;  

uniform sampler2DArray texture3;  // slope slopeN slopeColorMap
uniform vec2 textureScale3;  

uniform sampler2DArray texture4;  // base baseN baseColorMap
uniform vec2 textureScale4;  

#ifdef THEME_2
uniform sampler2DArray texture5;  
uniform vec2 textureScale5;  
	#ifdef THEME_3
	uniform sampler2DArray texture6;
	uniform vec2 textureScale6;  
		#ifdef THEME_4
		uniform sampler2DArray texture7;
		uniform vec2 textureScale7;  
			#ifdef THEME_5
			uniform sampler2DArray texture8;
			uniform vec2 textureScale8;  
			#endif
		#endif
	#endif
#endif  

smooth in vec3 norm;  
smooth in vec3 texCoord;  
in vec3 vertexWorld;  

uniform vec3 cameraWorld;  

#define LENGTH_MAX_TEX 70  
#define LENGTH_D_TEX 25  

layout(location=0) out vec4 outColor0;  

#ifdef DEFERRED  
   layout(location=1) out vec4 outColor1;  
   layout(location=2) out vec4 outColor2;  
#endif // DEFERRED  

float lenFactor;

vec4 read_theme(sampler2DArray tex, vec2 texC, vec2 scale)
{  
    vec4 near = texture(tex, vec3(texC*scale,0));
	vec4 far =  texture(tex, vec3(texC,2));
	vec4 product = near*far;
    return (near*0.5f + product*0.5f)*(1.f-lenFactor) + (far*0.5f + product*0.5f)*lenFactor;
}
 
vec4 read_theme_normal(sampler2DArray tex, vec2 texC, vec2 scale)
{  
    vec4 near = texture(tex, vec3(texC*scale,1));
	vec4 far =  vec4(0.5,0.5,1,1);
	vec4 product = near*far;
	//return near;
    return near*(1.f-lenFactor) + far*lenFactor;
}

void main()
{  
	lenFactor=length(cameraWorld-vertexWorld);
    lenFactor = clamp(lenFactor, LENGTH_MAX_TEX-LENGTH_D_TEX, LENGTH_MAX_TEX+LENGTH_D_TEX);
    lenFactor = (lenFactor-(LENGTH_MAX_TEX-LENGTH_D_TEX))/(2*LENGTH_D_TEX);
	
    vec2 texc=texCoord.xy;  
	
	vec3 texColor = read_theme(texture4, texc, textureScale3).xyz;
	vec3 slopeColor = read_theme(texture3, texc, textureScale3).xyz;
	
	#ifdef THEME_2
    vec3 mask=texture(texture2, texc*textureScale2).xyz;
	
    texColor =  (read_theme(texture5, texc, textureScale5).xyz*mask.x +
				 texColor*(1.f-mask.x));
	
		#ifdef THEME_3
		texColor =  (read_theme(texture6, texc, textureScale6).xyz*mask.y +
					 texColor*(1.f-mask.y));
			#ifdef THEME_4
			texColor =  (read_theme(texture7, texc, textureScale7).xyz*mask.z +
						 texColor*(1.f-mask.z));
				#ifdef THEME_5
				texColor =  (read_theme(texture8, texc, textureScale8).xyz*mask.w +
							 texColor*(1.f-mask.w));
				#endif
			#endif
		#endif
	#endif
	
	vec3 norm_in = normalize(texture(texture1, clamp(texCoord.xy, 0, 1)).xyz*2-1);
	
	const float B=0.45; const float DB=0.05;  
    float slope_coef=clamp(norm_in.z, B-DB, B+DB);  
    slope_coef=(slope_coef-B+DB)/(2*DB);  
    texColor = texColor*slope_coef + slopeColor*(1.f-slope_coef); 
				 
    outColor0=vec4(texColor, color.a); 
	
	#ifndef NORMAL_MAPPING
	outColor1 = vec4(norm_in*0.5+0.5, clamp(exponent/1024, 0, 1));
	#endif
	
#ifdef NORMAL_MAPPING
	vec3 texNormal = read_theme_normal(texture4, texc, textureScale4).xyz;
	slopeColor = read_theme_normal(texture3, texc, textureScale3).xyz;
	
	#ifdef THEME_2
    texNormal =  (read_theme_normal(texture5, texc, textureScale5).xyz*mask.x +
				  texNormal*(1.f-mask.x));
	
		#ifdef THEME_3
		texNormal =  (read_theme_normal(texture6, texc, textureScale6).xyz*mask.y +
					 texNormal*(1.f-mask.y));
			#ifdef THEME_4
			texNormal =  (read_theme_normal(texture7, texc, textureScale7).xyz*mask.z +
						 texNormal*(1.f-mask.z));
				#ifdef THEME_5
				texNormal =  (read_theme_normal(texture8, texc, textureScale8).xyz*mask.w +
							 texNormal*(1.f-mask.w));
				#endif
			#endif
		#endif
	#endif
	
	texNormal = texNormal*slope_coef + slopeColor*(1.f-slope_coef); 
	
	vec3 n = norm_in;
	vec3 t=vec3(1,0,0);
	mat3 tbn = mat3(t, cross(t,n), n);
	n = tbn*(texNormal*2-1);
	
	outColor1 = vec4(n*0.5+0.5, clamp(exponent/1024, 0, 1));
#endif

    outColor2 = material;  
}