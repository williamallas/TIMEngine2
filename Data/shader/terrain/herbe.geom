#version 330
 layout(points) in;
 layout(triangle_strip, max_vertices=16) out;
 
#define INSTANCING 128
  
#ifndef NO_MATRIX
 uniform mat4 projView;
 uniform vec3 cameraWorld;
#endif

uniform float time;
 
 flat in mat4 modelMat[];
 flat in vec3 t_normal[];
 smooth out vec3 in_normal;
 
#ifdef USE_TEXTURE
 out vec2 texCoord0;
 uniform sampler2D texture0;
 uniform sampler2D texture1;
 uniform vec4 userData[3];
#endif

#ifdef USE_DENSITY_MAP
 uniform sampler2D texture3;
#endif

 vec4 texture_bilinear(sampler2D tex, vec2 uv)
 {
     int textureSize = textureSize(tex,0).x;
     uv = clamp(uv, 0, 0.999999);
     uv = uv*(textureSize-1);
     ivec2 i_uv=ivec2(floor(uv.x), floor(uv.y));
     uv = fract(uv);

     vec4 tA = mix(texelFetch(tex, i_uv, 0),
                   texelFetch(tex, i_uv + ivec2(1,0), 0),
                   uv.x);
     vec4 tB = mix(texelFetch(tex, i_uv + ivec2(0,1), 0),
                   texelFetch(tex, i_uv + ivec2(1,1), 0),
                   uv.x);

     return mix(tA, tB, uv.y);
 }
 
 float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
 
 void emitTest1(vec3 offset, float sizeH, float sizeV)
 {
	vec3 worldPos = vec3(modelMat[0]*gl_in[0].gl_Position)+offset;
	
	vec2 coord = ((worldPos.xy - userData[0].z*userData[0].xy + userData[0].z*0.5)/userData[0].z);
	in_normal = normalize(texture_bilinear(texture1, coord).xyz*2-1);
	float f = in_normal.x; in_normal.x=in_normal.y; in_normal.y=f;
	
	if(in_normal.z < 0.45) return;

    worldPos.z = texture_bilinear(texture0, coord).r;
    worldPos.z *= userData[0].w;
	
	vec3 vecCam = worldPos - cameraWorld;
	
	float C=25;
	float A=userData[1].x;
	float B=userData[1].x-C;
	
	float l = length(worldPos - cameraWorld);
	if(l > B && l<A)
	{
		worldPos.z = worldPos.z-2*sizeV*((l-B)/C);
	}
	else if(l > A)
	{
		worldPos.z -= 2*sizeV;
	}
	
	vecCam = normalize(cross(vecCam, vec3(0,0,1)));
	vec2 tab[8] = vec2[](vec2(-sizeH, 0), vec2(sizeH, 0),
	                     vec2(-sizeH, 0.33*sizeV), vec2(sizeH, 0.33*sizeV),
				         vec2(-sizeH, 0.66*sizeV), vec2(sizeH, 0.66*sizeV),
				         vec2(-sizeH, sizeV), vec2(sizeH, sizeV));
						 
	vec2 tcoord[8] = vec2[](vec2(0,0), vec2(1,0), 
	                        vec2(0,0.33), vec2(1,0.33),
							vec2(0,0.66), vec2(1,0.66),
							vec2(0,1), vec2(1,1));
	
	float inSin=userData[1].w*time+userData[1].y+(worldPos.x+worldPos.y)*0.2;
	float off[8]=float[](0,0,sin(inSin)*0.25*userData[1].z, sin(inSin)*0.25*userData[1].z,
                           	 sin(inSin)*0.5*userData[1].z, sin(inSin)*0.5*userData[1].z,
							 sin(inSin)*userData[1].z, sin(inSin)*userData[1].z);
				   
	for(int i=0 ; i<8 ; ++i)
	{
		gl_Position=vec4(worldPos,1)+vec4(vecCam.xy*tab[i].x+vec2(off[i],0),tab[i].y,0);
		gl_Position=projView*gl_Position;
		texCoord0 = tcoord[i];
		EmitVertex();
	}
	EndPrimitive();
 }
 
 void emitAligned(vec3 offset, float sizeH, float sizeV)
 {
	vec3 worldPos = vec3(modelMat[0]*gl_in[0].gl_Position)+offset;
	
	vec2 coord = ((worldPos.xy - userData[0].z*userData[0].xy + userData[0].z*0.5)/userData[0].z);
	
#ifdef USE_DENSITY_MAP
	if(texture_bilinear(texture3, vec2(coord.x, coord.y)).x <= rand(worldPos.xy))
	{
		return;
	}
#endif
	
	in_normal = normalize(texture_bilinear(texture1, coord).xyz*2-1);
	//float f = in_normal.x; in_normal.x=in_normal.y; in_normal.y=f;
	
	if(in_normal.z < 0.45) return;

    worldPos.z = texture_bilinear(texture0, coord).r;
    worldPos.z *= userData[0].w;
	
	vec3 vecCam = worldPos - cameraWorld;
	
	float C=25;
	float A=userData[1].x;
	float B=userData[1].x-C;
	
	float l = length(worldPos - cameraWorld);
	if(l > B && l<A)
	{
		worldPos.z = worldPos.z-2*sizeV*((l-B)/C);
	}
	else if(l > A)
	{
		worldPos.z -= 2*sizeV;
	}
	
	vecCam = normalize(cross(vecCam, vec3(0,0,1)));
	vec2 tab[6] = vec2[](vec2(-sizeH, 0), vec2(sizeH, 0),
	                     vec2(-sizeH, 0.5*sizeV), vec2(sizeH, 0.5*sizeV),
				         vec2(-sizeH, sizeV), vec2(sizeH, sizeV));
						 
	vec2 tcoord[6] = vec2[](vec2(0,1), vec2(1,1), 
	                        vec2(0,0.5), vec2(1,0.5),
							vec2(0,0), vec2(1,0));
	
	float inSin=userData[1].w*time+userData[1].y+(worldPos.x+worldPos.y)*0.2;
	float off[6]=float[](0,0,
                         sin(inSin)*0.33*userData[1].z, sin(inSin)*0.33*userData[1].z,
						 sin(inSin)*userData[1].z, sin(inSin)*userData[1].z);
				   
	for(int i=0 ; i<6 ; ++i)
	{
		gl_Position=vec4(worldPos,1)+vec4(vecCam.xy*tab[i].x+vec2(off[i],0),tab[i].y,0);
		gl_Position=projView*gl_Position;
		texCoord0 = tcoord[i];
		EmitVertex();
	}
	EndPrimitive();
 }
 
 void emitStatic(vec3 offset, float sizeH, float sizeV, int crossDir)
 {
	vec3 worldPos = vec3(modelMat[0]*gl_in[0].gl_Position)+offset;
	
	vec2 coord = ((worldPos.xy - userData[0].z*userData[0].xy + userData[0].z*0.5)/userData[0].z);
	
#ifdef USE_DENSITY_MAP
	if(texture_bilinear(texture3, vec2(coord.x, coord.y)).x <= rand(worldPos.xy))
	{
		return;
	}
#endif
	
	in_normal = normalize(texture_bilinear(texture1, coord).xyz*2-1);
	//float f = in_normal.x; in_normal.x=in_normal.y; in_normal.y=f;
	
	if(in_normal.z < 0.45) return;

    worldPos.z = texture_bilinear(texture0, coord).r;
    worldPos.z *= userData[0].w;
	
	float C=25;
	float A=userData[1].x;
	float B=userData[1].x-C;
	
	float l = length(worldPos - cameraWorld);
	if(l > B && l<A)
	{
		worldPos.z = worldPos.z-2*sizeV*((l-B)/C);
	}
	else if(l > A)
	{
		worldPos.z -= 2*sizeV;
	}
	
	vec2 tab[6] = vec2[](vec2(-sizeH, 0), vec2(sizeH, 0),
	                     vec2(-sizeH, 0.5*sizeV), vec2(sizeH, 0.5*sizeV),
				         vec2(-sizeH, sizeV), vec2(sizeH, sizeV));
						 
	vec2 tcoord[6] = vec2[](vec2(0,1), vec2(1,1), 
	                        vec2(0,0.5), vec2(1,0.5),
							vec2(0,0), vec2(1,0));
	
	float inSin=userData[1].w*time+userData[1].y+(worldPos.x+worldPos.y)*0.2;
	float off[6]=float[](0,0,
                         sin(inSin)*0.33*userData[1].z, sin(inSin)*0.33*userData[1].z,
						 sin(inSin)*userData[1].z, sin(inSin)*userData[1].z);
	
	vec3 randXY  = vec3(rand(gl_in[0].gl_Position.xy+vec2(9960,45644))*2-1, 
	                    rand(gl_in[0].gl_Position.xy+vec2(-12354,1943216))*2-1,0);
	
	#ifdef NORMAL_ALIGNED
	vec3 dir = normalize(cross(in_normal, randXY));
	#else
	vec3 dir = normalize(randXY);
	#endif
	
	dir = crossDir==1 ? cross(in_normal, dir) : dir;
	
	for(int i=0 ; i<6 ; ++i)
	{
		#ifdef NORMAL_ALIGNED
	    vec3 d = dir*tab[i].x + in_normal*tab[i].y + vec3(0,1,0)*off[i];
		#else
		vec3 d = dir*tab[i].x + vec3(0,0,1)*tab[i].y + vec3(0,1,0)*off[i];
		#endif
		gl_Position=vec4(worldPos,1)+vec4(d,0);
		gl_Position=projView*gl_Position;
		texCoord0 = tcoord[i];
		EmitVertex();
	}
	EndPrimitive();
 }
 
 void emitStaticLow(vec3 offset, float sizeH, float sizeV, int crossDir)
 {
	vec3 worldPos = vec3(modelMat[0]*gl_in[0].gl_Position)+offset;
	
	vec2 coord = ((worldPos.xy - userData[0].z*userData[0].xy + userData[0].z*0.5)/userData[0].z);
	
#ifdef USE_DENSITY_MAP
	if(texture_bilinear(texture3, vec2(coord.x, coord.y)).x <= rand(worldPos.xy))
	{
		return;
	}
#endif
	
	in_normal = normalize(texture_bilinear(texture1, coord).xyz*2-1);
	//float f = in_normal.x; in_normal.x=in_normal.y; in_normal.y=f;
	
	if(in_normal.z < 0.45) return;

    worldPos.z = texture_bilinear(texture0, coord).r;
    worldPos.z *= userData[0].w;
	
	float C=min(25,userData[1].x/3);
	float A=userData[1].x;
	float B=userData[1].x-C;
	
	float l = length(worldPos - cameraWorld);
	if(l > B && l<A)
	{
		worldPos.z = worldPos.z-2*sizeV*((l-B)/C);
	}
	else if(l > A)
	{
		worldPos.z -= 2*sizeV;
	}
	
	vec2 tab[4] = vec2[](vec2(-sizeH, 0), vec2(sizeH, 0),
				         vec2(-sizeH, sizeV), vec2(sizeH, sizeV));
						 
	vec2 tcoord[4] = vec2[](vec2(0,1), vec2(1,1), 
							vec2(0,0), vec2(1,0));
	
	float inSin=userData[1].w*time+userData[1].y+(worldPos.x+worldPos.y)*0.2;
	float off[4]=float[](0,0, sin(inSin)*userData[1].z, sin(inSin)*userData[1].z);
	
	vec3 randXY  = vec3(rand(gl_in[0].gl_Position.xy+vec2(9960,45644))*2-1, 
	                    rand(gl_in[0].gl_Position.xy+vec2(-12354,1943216))*2-1,0);
	
	#ifdef NORMAL_ALIGNED
	vec3 dir = normalize(cross(in_normal, randXY));
	#else
	vec3 dir = normalize(randXY);
	#endif
	
	dir = crossDir==1 ? cross(in_normal, dir) : dir;
	
	for(int i=0 ; i<4 ; ++i)
	{
		#ifdef NORMAL_ALIGNED
	    vec3 d = dir*tab[i].x + in_normal*tab[i].y + vec3(0,1,0)*off[i];
		#else
		vec3 d = dir*tab[i].x + vec3(0,0,1)*tab[i].y + vec3(0,1,0)*off[i];
		#endif
		gl_Position=vec4(worldPos,1)+vec4(d,0);
		gl_Position=projView*gl_Position;
		texCoord0 = tcoord[i];
		EmitVertex();
	}
	EndPrimitive();
 }
 
 void main()
 {
	in_normal = t_normal[0];
	float sizeX = userData[2].x + rand(gl_in[0].gl_Position.xy+vec2(316850,914))*(userData[2].z-userData[2].x);
	float sizeY = userData[2].y + rand(gl_in[0].gl_Position.xy)*(userData[2].w-userData[2].y);

	#ifdef CAMERA_ALIGNED
		emitAligned(vec3(0,0,0), sizeX,sizeY);
	#else
		emitStaticLow(vec3(0,0,0), sizeX,sizeY,0);
		#ifdef CROSS_PATTERN
			emitStaticLow(vec3(0,0,0), sizeX,sizeY,1);
		#endif
	#endif
 }  