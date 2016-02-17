#version 430

#define USE_LIGHT
#define USE_DIR_SHADOW
#define LIGHT_BIAS 1.f/10.f
#define MAX_DLIGHT 10

layout(local_size_x=LS_X, local_size_y=LS_Y, local_size_z=1) in;

struct Light
{
	vec4 head; // (type, radius, x, x)
	vec4 position;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation;
    vec4 spotData;
};
layout(std140, binding = 0) buffer lightsBuffer { Light lights[]; };

uniform coherent writeonly image2D image0;

uniform sampler2D texture0; // color
uniform sampler2D texture1; // normal
uniform sampler2D texture2; // material
uniform sampler2D texture3; // depth
uniform sampler2D texture4; // depthTile

#ifdef USE_DIR_SHADOW
uniform sampler2DArrayShadow depthMapDirLight[MAX_DLIGHT];
#endif

uniform vec3 cameraWorld;
uniform vec3 cameraDir; 
uniform float cameraFov;
uniform float time;

uniform vec3 worldOrigin;
uniform mat4 worldOriginInvProjView;

uniform int nbLight;
uniform int nbDLight;

uniform vec4 dLightDirection[MAX_DLIGHT];
uniform vec4 dLightDiffuse[MAX_DLIGHT];
uniform vec4 dLightSpecular[MAX_DLIGHT];

#ifdef USE_DIR_SHADOW
uniform vec4 dLightShadowData[MAX_DLIGHT];

#define MAX_MATRIX_SHADOW 8
uniform mat4 dLightMatrixShadow[MAX_MATRIX_SHADOW];
#endif

uniform vec4 globalAmbient;

#define MAX_LIGHT_TILE 50
shared uint lightsTile[MAX_LIGHT_TILE];
shared uint lightsTileSize = 0;

vec3 computeFragPosWorldOrigin(float depth, vec2 texCoord)
{
	vec4 position = worldOriginInvProjView * vec4((texCoord.st-vec2(0.5))*2.0f, (depth-0.5)*2.0f, 1);
	return position.xyz / position.w;
}

vec4 computePlan(vec3 p1, vec3 p2, vec3 p3)
{
	vec3 n=normalize(cross(p1 - p2, p3 - p2));
	return vec4(n, -dot(n,p2));
}

vec4 computePlan(vec3 p, vec3 n)
{
	n = normalize(n);
	return vec4(n, -dot(n,p));
}

vec4 plans[6]; // left right top down near far

int collide(uint light)
{
	for(uint i=0 ; i<6 ; ++i)
	{
		if(dot(plans[i].xyz, lights[light].position.xyz - worldOrigin) + plans[i].w < -lights[light].head.y)
			return 0;
	}
	return 1;
}


float textureShadow(sampler2DArrayShadow tex, vec4 coord);

float textureShadow3x3(sampler2DArrayShadow tex, vec4 coord, vec2 sizeTex);

float textureShadow3x3C(sampler2DArrayShadow tex, vec4 coord, vec2 sizeTex);

float textureShadowPoisson5(sampler2DArrayShadow tex, vec4 coord, vec2 sizeTex);

void main()
{
	/***********************/
	/** BUILD DEPTH RANGE **/
	/***********************/
	
	vec4 color = texelFetch(texture0, ivec2(gl_GlobalInvocationID.xy),0);
	
#ifdef USE_LIGHT
	vec2 minMaxDepth = texelFetch(texture4, ivec2(gl_WorkGroupID.x, gl_WorkGroupID.y), 0).xy;
	if(minMaxDepth.x == 1)
	{
		//barrier();
		//groupMemoryBarrier();
		imageStore(image0, ivec2(gl_GlobalInvocationID.xy), color);
		return;
	}
#endif
	
#ifdef USE_LIGHT
	
	/*******************/
	/**BUILD TILE STEP**/
	/*******************/
	
	/** Compute frutum plan **/
	#define DEPTH_F -1
	vec3 tl = vec3(worldOriginInvProjView*vec4(-1,1,DEPTH_F,1));
    vec3 bl = vec3(worldOriginInvProjView*vec4(-1,-1,DEPTH_F,1));
    vec3 br = vec3(worldOriginInvProjView*vec4(1,-1,DEPTH_F,1));
	
	vec3 step_x = (br-bl) / gl_NumWorkGroups.x;
	vec3 step_y = (tl-bl) / gl_NumWorkGroups.y;
	
	vec3 tile_bl = bl + step_x*gl_WorkGroupID.x + step_y*gl_WorkGroupID.y;
	vec3 tile_br = bl + step_x*(gl_WorkGroupID.x+1) + step_y*gl_WorkGroupID.y;
	vec3 tile_tl = bl + step_x*gl_WorkGroupID.x + step_y*(gl_WorkGroupID.y+1);
	vec3 tile_tr = bl + step_x*(gl_WorkGroupID.x+1) + step_y*(gl_WorkGroupID.y+1);
	
	plans[0] = computePlan(vec3(0),tile_tl,tile_bl);
	plans[1] = computePlan(vec3(0),tile_br,tile_tr);
	plans[2] = computePlan(vec3(0),tile_tr,tile_tl);
	plans[3] = computePlan(vec3(0),tile_bl,tile_br);
	
	vec4 pts_near = worldOriginInvProjView*vec4(0,0,minMaxDepth.x*2-1,1);
	vec4 pts_far = worldOriginInvProjView*vec4(0,0,minMaxDepth.y*2-1,1);
	plans[4] = computePlan(pts_near.xyz/pts_near.w, cameraDir);
	plans[5] = computePlan(pts_far.xyz/pts_far.w, -cameraDir);
	
	uint localRange = 1 + (nbLight / (LS_X*LS_Y));

	uint start = gl_LocalInvocationIndex * localRange;
	uint end = min(start+localRange, nbLight);
	
	uint safe_l=0;

	for(uint i=start ; i<end && safe_l < MAX_LIGHT_TILE ; ++i)
	{
		if(collide(i) == 1)
		{
		    safe_l = atomicAdd(lightsTileSize, 1);
			lightsTile[safe_l] = i;
		}
	}
#endif
	/*******************/
	/** LIGHTING STEP **/
	/*******************/
	
	vec2 sCoord =  vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y)/vec2(gl_NumWorkGroups.x*LS_X, gl_NumWorkGroups.y*LS_Y);
	float depth = texelFetch(texture3, ivec2(gl_GlobalInvocationID.xy),0).r;
	
	if(depth == 1)
	{
		#ifdef USE_LIGHT
		barrier();
		#endif
		imageStore(image0, ivec2(gl_GlobalInvocationID.xy), color);
		return;
	}

	#ifdef USE_DIR_SHADOW
	vec3 fragPos = computeFragPosWorldOrigin(depth, sCoord);
	#endif
	
	vec4 normal_frag = texelFetch(texture1, ivec2(gl_GlobalInvocationID.xy),0);
	vec4 material = texelFetch(texture2, ivec2(gl_GlobalInvocationID.xy),0);
	
	vec3 normal = normalize(normal_frag.xyz*2-1);
	
	vec4 finalColor = globalAmbient * color * material.x;
	
#ifdef USE_LIGHT
	barrier();
	if(lightsTileSize > 0)
	{	
		for(int i=0 ; i<lightsTileSize ; ++i)
		{
			vec3 lightDir =(lights[lightsTile[i]].position.xyz - worldOrigin - fragPos);
			vec4 attenuation = lights[lightsTile[i]].attenuation;
			float dist = max(0,length(lightDir) - attenuation.w);
			
			lightDir = normalize(lightDir);
			
			// Diffuse 
			float att = 1.0 / (dist * dist * attenuation.z + dist * attenuation.y + attenuation.x) - LIGHT_BIAS;
			att = max(0, att);

			float lightCoef = clamp(dot(lightDir, normal),0,1);
			
			finalColor += (lightCoef * color * material.y * lights[lightsTile[i]].diffuse * att);
			
			// Specular
			/*vec3 R = reflect(-lightDir, normal);
			int shininess = int(normal_frag.w * 1024 + 0.5);
			vec3 vertexDir = normalize(-fragPos);
		
			lightCoef = pow(max(dot(R, vertexDir),0), shininess);
			finalColor += ( lightCoef * color * material.z * lights[lightsTile[i]].specular * att);*/
		}
	}
#endif
	
	int indexProjMatrix=0;
	for(int i=0 ; i<nbDLight ; ++i)
	{
		float shadowCoef = 1;
	
	#ifdef USE_DIR_SHADOW
		/*vec3 coord = (dLightMatrixShadow[0] * vec4(fragPos,1)).xyz;
		if(coord.y <= 1 && coord.y >= 0 && coord.x <= 1 && coord.x >= 0 && coord.z <= 1 && coord.z >= 0)
			shadowCoef = textureShadow(depthMapDirLight[i], vec4(coord.xy, 0, coord.z)) * (1.f-dLightShadowData[i].x) + dLightShadowData[i].x;
		else
		{
			coord = (dLightMatrixShadow[1] * vec4(fragPos,1)).xyz;
			if(coord.y <= 1 && coord.y >= 0 && coord.x <= 1 && coord.x >= 0 && coord.z <= 1 && coord.z >= 0)
				shadowCoef = textureShadow(depthMapDirLight[i], vec4(coord.xy, 1, coord.z)) * (1.f-dLightShadowData[i].x) + dLightShadowData[i].x;
			else
			{
				coord = (dLightMatrixShadow[2] * vec4(fragPos,1)).xyz;
				if(coord.y <= 1 && coord.y >= 0 && coord.x <= 1 && coord.x >= 0 && coord.z <= 1 && coord.z >= 0)
					shadowCoef = textureShadow(depthMapDirLight[i], vec4(coord.xy, 2, coord.z)) * (1.f-dLightShadowData[i].x) + dLightShadowData[i].x;
			}
		}*/
	#endif
		
	#ifdef USE_DIR_SHADOW
		int nbLevel = int(dLightShadowData[i].y+0.5);
		int res = int(dLightShadowData[i].z+0.5);
		const float O=0.01;
		for(int j=0 ; j<nbLevel ; ++j)
		{
			vec3 coord = (dLightMatrixShadow[indexProjMatrix++] * vec4(fragPos,1)).xyz;
			
			if(coord.y <= 1-O && coord.y >= 0+O && coord.x <= 1-O && coord.x >= 0+O && coord.z <= 1-O && coord.z >= 0+O)
			{
				shadowCoef = textureShadow(depthMapDirLight[i], vec4(coord.xy, j, coord.z)) * (1.f-dLightShadowData[i].x) + dLightShadowData[i].x;
				//lightCoef *= textureShadow3x3(depthMapDirLight[i], vec4(coord.xy, j, coord.z), vec2(res,res)) * (1.f-dLightShadowData[i].x) + dLightShadowData[i].x;
				//lightCoef *= textureShadow3x3C(depthMapDirLight[i], vec4(coord.xy, j, coord.z), vec2(res,res)) * (1.f-dLightShadowData[i].x) + dLightShadowData[i].x;
				//lightCoef *= textureShadowPoisson5(depthMapDirLight[i], vec4(coord.xy, j, coord.z), vec2(res,1200)) * (1.f-dLightShadowData[i].x) + dLightShadowData[i].x;
				//color *= vec4(j==0?1:0.8, j==1?1:0.8,j==2?1:0.8,1);
				indexProjMatrix += (nbLevel-j-1);
				break;
			}
		}
	#endif
		float coef = clamp(dot(-dLightDirection[i].xyz, normal),0,1);
		finalColor += (shadowCoef * coef * color * material.y * dLightDiffuse[i]);
		
		vec3 R = reflect(dLightDirection[i].xyz, normal);
		int shininess = int(normal_frag.w * 1024 + 0.5);
		vec3 vertexDir = normalize(-fragPos);
		
		coef = pow(max(dot(R, vertexDir),0), shininess);
		finalColor += ( shadowCoef * coef * color * material.z * dLightSpecular[i]);
	}
	
	/** Emissive **/
	finalColor += color * material.w * 5;
	imageStore(image0, ivec2(gl_GlobalInvocationID.xy), finalColor);
}

const float BIAS[] = {0.0003, 0.0005, 0.0010 };
float textureShadow(sampler2DArrayShadow tex, vec4 coord)
{
	coord.w -= BIAS[int(coord.z+0.5)];
	return texture(tex, coord);
}

float textureShadow3x3(sampler2DArrayShadow tex, vec4 coord, vec2 sizeTex)
{
	//coord.w -= BIAS[int(coord.z+0.5)];
	float res = texture(tex, coord);
	const float o_clamp = 1.f/sizeTex.x;
	const float o = 1.f/sizeTex.y;
	
	vec2 dx = vec2(clamp(coord.x+o,o_clamp,1.f-o_clamp), clamp(coord.x-o,o_clamp,1.f-o_clamp));
	vec2 dy = vec2(clamp(coord.y+o,o_clamp,1.f-o_clamp), clamp(coord.y-o,o_clamp,1.f-o_clamp));
	
	res += texture(tex, vec4(dx.x, coord.yzw))*0.5;
	res += texture(tex, vec4(dx.y, coord.yzw))*0.5;
	res += texture(tex, vec4(coord.x, dy.x, coord.zw))*0.5;
	res += texture(tex, vec4(coord.x, dy.y, coord.zw))*0.5;
	
	res += texture(tex, vec4(dx.x, dy.x, coord.zw))*0.25;
	res += texture(tex, vec4(dx.y, dy.x, coord.zw))*0.25;
	res += texture(tex, vec4(dx.x, dy.y, coord.zw))*0.25;
	res += texture(tex, vec4(dx.y, dy.y, coord.zw))*0.25;
	
	return res*0.25; 
}

float textureShadow3x3C(sampler2DArrayShadow tex, vec4 coord, vec2 sizeTex)
{
	coord.w -= BIAS[int(coord.z+0.5)];
	float res = texture(tex, coord);
	const float o_clamp = 1.f/sizeTex.x;
	const float o = 1.f/sizeTex.y;
	
	vec2 dx = vec2(clamp(coord.x+o,o_clamp,1.f-o_clamp), clamp(coord.x-o,o_clamp,1.f-o_clamp));
	vec2 dy = vec2(clamp(coord.y+o,o_clamp,1.f-o_clamp), clamp(coord.y-o,o_clamp,1.f-o_clamp));
	
	res += texture(tex, vec4(dx.x, coord.yzw));
	res += texture(tex, vec4(dx.y, coord.yzw))*0.5;
	res += texture(tex, vec4(coord.x, dy.x, coord.zw));
	res += texture(tex, vec4(coord.x, dy.y, coord.zw));
	
	res += texture(tex, vec4(dx.x, dy.x, coord.zw));
	res += texture(tex, vec4(dx.y, dy.x, coord.zw));
	res += texture(tex, vec4(dx.x, dy.y, coord.zw));
	res += texture(tex, vec4(dx.y, dy.y, coord.zw));
	
	return res/9; 
}

float textureShadowPoisson5(sampler2DArrayShadow tex, vec4 coord, vec2 sizeTex)
{
	coord.w -= BIAS[int(coord.z+0.5)];
	
	vec2 poissonDisk[4] = vec2[](
		vec2( -0.94201624, -0.39906216 ),
		vec2( 0.94558609, -0.76890725 ),
		vec2( -0.094184101, -0.92938870 ),
		vec2( 0.34495938, 0.29387760 )
	);
	
	const float o_clamp = 1.f/sizeTex.x;
	const float o = 1.f/sizeTex.y;
	
	for (int i=0 ; i<4 ; ++i)
	{
		poissonDisk[i] = vec2(clamp(poissonDisk[i].x*o+coord.x, o_clamp,1.f-o_clamp), 
							  clamp(poissonDisk[i].y*o+coord.y, o_clamp,1.f-o_clamp));
	}
	
	float col = texture(tex, coord);
	for (int i=0 ; i<4 ; ++i)
	{
		col += texture(tex, vec4(poissonDisk[i].xy, coord.zw));
	}
		
	return col * 0.2;
}