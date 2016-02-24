#version 430

#define LIGHT_BIAS 0.1f
#define LS_X 32
#define LS_Y 30

layout(local_size_x=LS_X, local_size_y=LS_Y, local_size_z=1) in;

layout(std140, binding = 0) uniform DrawParameter
{
	mat4 view, proj;
	mat4 projView, invView, invProj, invProjView, worldOriginInvProjView;
	vec4 cameraPos, cameraUp, cameraDir, worldOrigin;
	vec4 time;
}; 

struct Light
{
	vec4 head; // (type, radius, power, x)
	vec4 position;
    vec4 color;
};
layout(std140, binding = 0) buffer lightsBuffer { Light lights[]; };

uniform writeonly image2D image0;

uniform sampler2D texture0; // color
uniform sampler2D texture1; // normal
uniform sampler2D texture2; // material
uniform sampler2D texture3; // depth
uniform sampler2D texture4; // depthTile

uniform int nbLight;

// #define MAX_LIGHT_TILE 200
// shared uint lightsTile[MAX_LIGHT_TILE];
// shared uint lightsTileSize = 0;

// vec3 computeFragPosWorldOrigin(float depth, vec2 texCoord)
// {
	// vec4 position = worldOriginInvProjView * vec4((texCoord.st-vec2(0.5))*2.0, (depth-0.5)*2.0, 1);
	// return position.xyz / position.w;
// }

// vec3 computeFragPos(float depth, vec2 texCoord)
// {
	// vec4 position = invProjView * vec4((texCoord.st-vec2(0.5))*2.0, (depth-0.5)*2.0, 1);
	// return position.xyz / position.w;
// }


// vec4 computePlan(vec3 p1, vec3 p2, vec3 p3)
// {
	// vec3 n=normalize(cross(p1 - p2, p3 - p2));
	// return vec4(n, -dot(n,p2));
// }

// vec4 computePlan(vec3 p, vec3 n)
// {
	// n = normalize(n);
	// return vec4(n, -dot(n,p));
// }

// vec4 plans[6]; // left right top down near far

// int collide(uint light)
// {
	// for(uint i=0 ; i<6 ; ++i)
	// {
		// if(dot(plans[i].xyz, lights[light].position.xyz - worldOrigin) + plans[i].w < -lights[light].head.y)
			// return 0;
	// }
	// return 1;
// }

float distPlan(vec4 plan, vec3 p)
{
	return dot(plan.xyz, p) + plan.w;
}

void main()
{
	/*ivec2 texSize = imageSize(image0);
	if(gl_GlobalInvocationID.x >= texSize.x || gl_GlobalInvocationID.y >= texSize.y)
		return;*/
	
/* 	vec4 color = texelFetch(texture0, ivec2(gl_GlobalInvocationID.xy),0);
	vec2 minMaxDepth = texelFetch(texture4, ivec2(gl_WorkGroupID.x, gl_WorkGroupID.y), 0).xy;
	if(minMaxDepth.x == 1)
	{
		imageStore(image0, ivec2(gl_GlobalInvocationID.xy), color);
		return;
	} */
	
	/*******************/
	/**BUILD TILE STEP**/
	/*******************/
	
	/** Compute frutum plan **/
	/* #define DEPTH_F -1
	vec3 tl = vec3(worldOriginInvProjView*vec4(-1,1,DEPTH_F,1));
    vec3 bl = vec3(worldOriginInvProjView*vec4(-1,-1,DEPTH_F,1));
    vec3 br = vec3(worldOriginInvProjView*vec4(1,-1,DEPTH_F,1));
	
	vec3 step_x = (br-bl) / gl_NumWorkGroups.x;
	vec3 step_y = (tl-bl) / gl_NumWorkGroups.y;
	
	vec3 tile_bl = bl + step_x*gl_WorkGroupID.x + step_y*gl_WorkGroupID.y;
	vec3 tile_br = bl + step_x*(gl_WorkGroupID.x+1) + step_y*gl_WorkGroupID.y;
	vec3 tile_tl = bl + step_x*gl_WorkGroupID.x + step_y*(gl_WorkGroupID.y+1);
	vec3 tile_tr = bl + step_x*(gl_WorkGroupID.x+1) + step_y*(gl_WorkGroupID.y+1);
	
	plans[0] = computePlan(cameraWorld-worldOrigin,tile_tl,tile_bl);
	plans[1] = computePlan(cameraWorld-worldOrigin,tile_br,tile_tr);
	plans[2] = computePlan(cameraWorld-worldOrigin,tile_tr,tile_tl);
	plans[3] = computePlan(cameraWorld-worldOrigin,tile_bl,tile_br);
	
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
 */
	/*******************/
	/** LIGHTING STEP **/
	/*******************/
	
	/* vec2 sCoord =  vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y)/vec2(gl_NumWorkGroups.x*LS_X, gl_NumWorkGroups.y*LS_Y);
	float depth = texelFetch(texture3, ivec2(gl_GlobalInvocationID.xy),0).r;
	
	if(depth == 1)
	{
		memoryBarrierShared();
		barrier();
		imageStore(image0, ivec2(gl_GlobalInvocationID.xy), color);
		return;
	}
	
	vec3 fragPos = computeFragPosWorldOrigin(depth, sCoord);
	
	vec4 normal_frag = texelFetch(texture1, ivec2(gl_GlobalInvocationID.xy),0);
	vec4 material = texelFetch(texture2, ivec2(gl_GlobalInvocationID.xy),0);
	
	vec3 normal = normalize(normal_frag.xyz*2-1);
	
	vec4 finalColor = vec4(0);
	
	//groupMemoryBarrier();
	memoryBarrierShared();
	barrier();
	
	
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
		vec3 R = reflect(-lightDir, normal);
		int packed_exp_ref = int(normal_frag.w*256*256);
		float shininess = float(packed_exp_ref & 0xff);

		vec3 vertexDir = normalize(-fragPos);
	
		lightCoef = pow(max(dot(R, vertexDir),0), shininess);
		finalColor += ( lightCoef * color * material.z * lights[lightsTile[i]].specular * att);
	} */

	imageStore(image0, ivec2(gl_GlobalInvocationID.xy), vec4(gl_GlobalInvocationID.x/1900.f, gl_GlobalInvocationID.y/1100.f, 0, 1));
	
}