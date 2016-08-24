#version 430
#define NB_MIPMAP 7
 
precision highp float;
const float PI = 3.14159265359;

//uniform sampler2D textures[7];
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

uniform samplerCube texture4; // skybox
uniform samplerCube texture5; // blurred skybox
uniform sampler2D texture6; // brdf
uniform sampler2D texture7; // reflexion

uniform sampler2DArrayShadow texture8;

uniform int enableGI;
uniform vec4 globalAmbient;
uniform int localReflexion;

#define MAX_LIGHTS 8
#define MAX_SHADOW_LVL 4
uniform int nbLights;
uniform vec4 lightDirection[MAX_LIGHTS];
uniform vec4 lightColor[MAX_LIGHTS];
uniform mat4 lightMatrix[MAX_SHADOW_LVL];

in vec2 coord;

layout(location=0) out vec4 outColor; 

layout(std140, binding = 0) uniform DrawParameter
{
	mat4 view, proj;
	mat4 projView, invView, invProj, invProjView, worldOriginInvProjView;
	vec4 cameraPos, cameraUp, cameraDir, worldOrigin;
	vec4 time;
}; 

vec3 computeFragPos(float depth, vec2 texCoord)
{
	vec4 position = invProjView * vec4((texCoord.st-vec2(0.5))*2.0f, (depth-0.5)*2.0f, 1);
	return position.xyz / position.w;
}

vec3 unproj(vec2 texCoord, float depth)
{
	vec4 position = invProj	* vec4((texCoord.st-vec2(0.5))*2.0f, (depth-0.5)*2.0f, 1);
	return position.xyz / position.w;
}

vec3 unprojWorld(vec2 texCoord, float depth)
{
	vec4 position = invProjView	* vec4((texCoord.st-vec2(0.5))*2.0f, (depth-0.5)*2.0f, 1);
	return position.xyz / position.w;
}

vec3 project(vec3 pos)
{
	vec4 position = proj * vec4(pos, 1);
	return position.xyz / position.w;
}

float D_GGX(float rough, float dotNH)
{
	float rough2 = rough*rough;
	float alpha2 = rough2 * rough2;
	float deno = dotNH*dotNH*(alpha2-1)+1;
	return alpha2 / (PI*deno*deno);
}

float helper_G_smith(float dotVal, float k)
{
	return dotVal / (dotVal*(1-k)+k);
}

float G_smith(float dotNV, float dotNL, float roughness)
{
	float k = (roughness+1)*(roughness+1) / 8;
    return helper_G_smith(dotNL,k)*helper_G_smith(dotNV,k);
}

float F_shlick(float F0, float dotVH)
{
	return F0+(1-F0)*pow((1-dotVH),5);
}

float textureShadow3x3(sampler2DArrayShadow t, vec4 c, vec2 s);

vec4 raytraceCamSpace(vec3 rayPos, vec3 rayDir, sampler2D ZBuffer, vec3 n, vec4 backColor);

vec3 approximateSpecular(vec3 specColor, float roughness, vec3 r, float dotNV);

void main()
{  
	outColor = vec4(0,0,0,0);
	
	float depth = texture(texture3, coord).r;
	vec4 normal_frag = texture(texture1, coord);
	vec4 material = texture(texture2, coord); // roughness, metallic, specular, emissive 
	vec4 color = texture(texture0, coord);
	vec3 normal = normalize(normal_frag.xyz*2-1);
	vec3 frag_pos = computeFragPos(depth, coord);
	
	float roughness = material.x;
	float metallic = material.y;
	
	vec3 viewDir = normalize(frag_pos - vec3(cameraPos));
	
	if(material.x==1 && material.y==1)
	{
		return;
	}
	if(depth == 1)
	{
		outColor = texture(texture4, viewDir);
		outColor.a=0;
		return;
	}
	else if(roughness < 0.01 && localReflexion == 1) // raytrace camspace
	{
		vec3 world = unproj(coord, depth);
		vec3 I = normalize(world);
		vec3 n =  mat3(view) * normalize(normal_frag.xyz*2-1);
		vec3 reflected = normalize(reflect(I, n));
		
		vec4 specColor = mix(vec4(material.z), color, material.y);
		outColor = specColor*raytraceCamSpace(world, reflected, texture3, n, texture(texture4, reflect(viewDir, normal)));
	}
	else if(enableGI == 1)
	{	
		vec4 diffuseTerm = color * (1-material.y) * textureLod(texture5, normal,NB_MIPMAP-1);
		
		vec3 specularColor = mix(vec3(material.z), vec3(color), metallic);
		vec3 specTerm = approximateSpecular(specularColor ,roughness, reflect(viewDir, normal), max(dot(normal, -viewDir), 0.01));
		
		outColor.rgb = specTerm + vec3(diffuseTerm);
	}
	else
	{
		vec4 diffuseTerm = color * (1-material.y) * globalAmbient;
		vec2 a_b = texture(texture6, vec2(max(dot(normal, -viewDir), 0.01), roughness)).xy;
		
		vec3 specularColor = mix(vec3(material.z), vec3(color), metallic);
		vec3 specTerm = globalAmbient.rgb * (specularColor*a_b.x + vec3(a_b.y));
		
		outColor.rgb = specTerm + vec3(diffuseTerm);
	}
	
	outColor += vec4(color * material.w * 5);
	float dotNV = clamp(dot(normal, -viewDir), 0.0001, 0.9999);
		
	for(int i=0 ; i<nbLights ; ++i)
	{
		vec3 L = -normalize(lightDirection[i].xyz);
		vec3 H = normalize(L - viewDir);
		
		float dotNL = dot(normal, L);
		float dotNH = dot(normal, H);
		float dotVH = dot(-viewDir,H);
		
		if(dotNL <= 0 || dotNV <= 0) continue;
		
		dotNL = clamp(dotNL, 0.0001,0.9999);
		dotNH = clamp(dotNH, 0.0001,0.9999);
		dotVH = clamp(dotVH, 0.0001,0.9999);
		
		float G_CT = G_smith(dotNV, dotNL, roughness);
		float F_CT = F_shlick(mix(0.04, 0.9, metallic), dotVH);
		float D_CT = D_GGX(roughness, dotNH);
		
		float specular = D_CT*G_CT*F_CT / (4*dotNV*dotNL);
		
		vec3 diffuseColor = color.xyz - color.xyz * metallic;
		vec3 specularColor = mix(vec3(material.z), color.xyz, metallic);
		
		float shadowCoef = 1;
		
		//for(float s=0 ; s<lightDirection[i].w ; s+=1)
		if(lightDirection[i].w > 0)
		{
			ivec3 tSize = textureSize(texture8,0);
			for(int layer=0 ; layer<min(MAX_SHADOW_LVL, tSize.z) ; ++layer)
			{
				vec4 s_coord = (lightMatrix[layer] * vec4(frag_pos,1));
				s_coord /= s_coord.w;
				
				if(s_coord.x >= 0 && s_coord.x <= 1 && s_coord.y >= 0 && s_coord.y <= 1 && s_coord.z >= 0 && s_coord.z <= 1)
				{
					//shadowCoef = texture(textures[i], vec4(s_coord.xy, float(layer), s_coord.z));
					shadowCoef = textureShadow3x3(texture8, vec4(s_coord.xy, float(layer), s_coord.z), vec2(tSize.x, tSize.y));
					break;
				}
			}
		}
		
		outColor.xyz += shadowCoef * dotNL * lightColor[i].xyz * (diffuseColor + specularColor*specular);
	}
	
	outColor.a = 0;
}

vec3 approximateSpecular(vec3 specColor, float roughness, vec3 r, float dotNV)
{
	const float rough[NB_MIPMAP] = {0, 0.05, 0.13, 0.25, 0.45, 0.66, 1};
	int index=NB_MIPMAP-2;
	for(int i=0 ; i<NB_MIPMAP-1 ; ++i)
	{
		if(rough[i] >= roughness)
		{
			index=i;
			break;
		}
	}
	
	float coef = (roughness - rough[index]) / (rough[index+1] - rough[index]);
	
	vec2 a_b = texture(texture6, vec2(dotNV, roughness)).xy;
	
	return textureLod(texture5, r, float(index)+coef).rgb * (specColor*a_b.x+vec3(a_b.y));
}

bool traceScreenSpaceRay(vec3 csOrig, vec3 csDir, sampler2D csZBuffer, vec2 csZBufferSize, 
						 float zThickness, const bool csZBufferIsHyperbolic, vec3 clipInfo, float nearPlaneZ,
				         float stride, float jitter, const float maxSteps, float maxDistance,
						 out vec2 hitPixel, out vec3 csHitPoint) 
{

	 // Clip to the near plane
	 float rayLength = ((csOrig.z + csDir.z * maxDistance) > nearPlaneZ) ? (nearPlaneZ - csOrig.z) / csDir.z : maxDistance;
	 vec3 csEndPoint = csOrig + csDir * rayLength;
	 hitPixel = vec2(-1, -1);

	 // Project into screen space
	 vec4 H0 = proj * vec4(csOrig, 1.0), H1 = proj * vec4(csEndPoint, 1.0);
	 float k0 = 1.0 / H0.w, k1 = 1.0 / H1.w;
	 vec3 Q0 = csOrig * k0, Q1 = csEndPoint * k1;

	 // Screen-space endpoints
	 vec2 P0 = csZBufferSize.x*(H0.xy*0.5+0.5) * k0, P1 = csZBufferSize.y*(H1.xy*0.5+0.5) * k1;

	 // [ Optionally clip here using listing 4 ]

	 P1 += vec2((dot(P0-P1, P0-P1) < 0.0001) ? 0.01 : 0.0);
	 vec2 delta = P1 - P0;

	 bool permute = false;
	 if (abs(delta.x) < abs(delta.y)) {
		permute = true;
		delta = delta.yx; P0 = P0.yx; P1 = P1.yx;
	 }

	 float stepDir = sign(delta.x), invdx = stepDir / delta.x;

	 // Track the derivatives of Q and k.
	 vec3 dQ = (Q1 - Q0) * invdx;
	 float dk = (k1 - k0) * invdx;
	 vec2 dP = vec2(stepDir, delta.y * invdx);

	dP *= stride; dQ *= stride; dk *= stride;
	P0 += dP * jitter; Q0 += dQ * jitter; k0 += dk * jitter;
	float prevZMaxEstimate = csOrig.z;
 
	 // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
	 vec3 Q = Q0; float k = k0, stepCount = 0.0, end = P1.x * stepDir;
	for (vec2 P = P0; ((P.x * stepDir) <= end) && (stepCount < maxSteps); P += dP, Q.z += dQ.z, k += dk, stepCount += 1.0) 
	{
		// Project back from homogeneous to camera space
		hitPixel = permute ? P.yx : P;

		// The depth range that the ray covers within this loop iteration.
		// Assume that the ray is moving in increasing z and swap if backwards.
		float rayZMin = prevZMaxEstimate;
		// Compute the value at 1/2 pixel into the future
		float rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
		prevZMaxEstimate = rayZMax;
		if (rayZMin > rayZMax) 
		{ 
			float tmp = rayZMin;
			rayZMin = rayZMax;
			rayZMax=tmp;
			//swap(rayZMin, rayZMax);
		}

		// Camera-space z of the background
		float sceneZMax = texelFetch(csZBuffer, ivec2(hitPixel.x, hitPixel.y), 0).r;

		/*if (csZBufferIsHyperbolic)
			sceneZMax = reconstructCSZ(sceneZMax, clipInfo);*/

		float sceneZMin = sceneZMax - zThickness;

		if (((rayZMax >= sceneZMin) && (rayZMin <= sceneZMax)) || (sceneZMax == 0))
		{
			break;
		}

	} // for each pixel on ray

	 // Advance Q based on the number of steps
	 Q.xy += dQ.xy * stepCount; csHitPoint = Q * (1.0 / k);
	 return all(lessThanEqual(abs(hitPixel - (csZBufferSize * 0.5)), csZBufferSize * 0.5));
}

bool raytraceDepth(vec3 rayPos, vec3 rayDir, sampler2D ZBuffer, out vec2 intersectCoord)
{
	const float maxDist = 2.0;
	const float maxStep = 30.0;
	ivec2 ibufferSize = textureSize(ZBuffer,0);
	vec2 bufSize = vec2(ibufferSize.x, ibufferSize.y);
	
	vec3 P0 = rayPos, P1 = rayPos + rayDir * maxDist;
	
	vec4 hoP0 = proj * vec4(P0,1), hoP1 = proj * vec4(P1,1);

	float z0 = 1.f / hoP0.w, z1 = 1.f / hoP0.w;
	float dz = z1-z0;
	
	vec3 Q0 = P0 * z0, Q1 = P1 * z1;
	vec3 dQ = Q1-Q0;
	
	vec4 dHoP = hoP1 - hoP0;
	bool swapXY=false;
	if(abs(dHoP.x) < abs(dHoP.y))
	{
		swapXY=true;
		dHoP.yx = dHoP.xy; hoP0.xy = hoP0.yx; hoP1.xy = hoP1.yx;
	}
	
	vec2 sP0 = hoP0.xy * z0, sP1 = hoP1.xy * z1;
	vec2 dsP = sP1-sP0;
	//float sign_dsP = sign(dsP.x);
	float invx = 3.0 / (dsP.x*bufSize.x*0.5);
	//invx = 1.f/maxStep;
	
	dsP *= invx;
	dQ *= invx; 
	dz *= invx;
	dHoP *= invx;
	
	vec2 sP=sP0; vec4 hoP=hoP0;
	vec3 Q = Q0;
	float prevDepth = 1;
	
	for(float nbStep=0, curZ = z0; nbStep < maxStep ; nbStep+=1.0, curZ += dz, sP+=dsP, Q+=dQ, hoP+=dHoP)
	{
		float curRayDepthCS = hoP.z / hoP.w;
		curRayDepthCS = curRayDepthCS*0.5+0.5;
		vec2 coord = (swapXY ? sP.yx : sP.xy)*0.5+0.5;
		float curBackDepth = texture(ZBuffer, coord).r;
		//curBackDepth = unproj(coord, curBackDepth).z;
		
		if(curRayDepthCS >= curBackDepth && prevDepth <= curBackDepth)
		{
			intersectCoord = coord;
			return true;
			//break;
		}
		
		prevDepth = curRayDepthCS;
	}

	
	return false;
}

vec4 binarySearch(vec3 noP, vec3 yesP, sampler2D ZBuffer, out vec3 hitP)
{
	vec3 m, result=yesP;
	
	for(int i=0 ; i<10 ; ++i)
	{
		m = (noP+yesP)*0.5;
		vec3 proj_P = project(m)*0.5+0.5;
		float d = texture(ZBuffer, proj_P.xy).r;
		
		if(proj_P.z > d) yesP = m;
		else noP = m;
		
		result = yesP;
	}

	result = project(yesP)*0.5+0.5;
	hitP = vec3(yesP.xy, unproj(result.xy, texture(ZBuffer, result.xy).r).z);
	return texture(texture0, result.xy);
}

vec4 computePlan(vec3 p, vec3 n)
{
	n = normalize(n);
	return vec4(n, -dot(n,p));
}

float dist(vec4 plan, vec3 p)
{
	return dot(plan.xyz, p) + plan.w;
} 

vec4 raytraceCamSpace(vec3 rayPos, vec3 rayDir, sampler2D ZBuffer, vec3 n, vec4 backColor)
{
	const float maxDist = 8;
	const float maxStep = 15.0;
	
	//float d_e = dist(computePlan(rayPos, n), rayPos+rayDir);
	
	vec3 rayStep = rayDir;// * (maxDist / maxStep);
	rayStep /= length(rayStep.xy);
	rayStep *= (maxDist/maxStep);
	vec3 P=rayPos;
	float step=0;
	vec3 prevP=vec3(rayPos);
	
	for(; step<maxStep ; step+=1, P+=rayStep)
	{
		vec3 curP = P+rayStep*0.5;
		vec3 proj_P = project(curP);
		if(!all(lessThanEqual(abs(proj_P), vec3(1.f))))
			return backColor;
		
		proj_P = proj_P*0.5+0.5;
		float d = texture(ZBuffer, proj_P.xy).r;
		
		if(proj_P.z > d) // LE NEAR = 0 le FAR = -inf
		{
			vec3 hitP;
			vec4 res = binarySearch(prevP, curP, ZBuffer, hitP);
			
			vec3 hitVec = normalize(hitP - rayPos);
			if(dot(rayDir, hitVec) > 1.f-0.001)
			{
				float c = 1.f-clamp((dist(computePlan(rayPos, n), hitP)-(maxDist*0.4)) / (maxDist*0.4),0,1);
				return res*c + backColor*(1.f-c);
			}
		}
		else prevP = curP;
		
		rayStep *= 1;
	}
	
	return backColor;
}

const float BIAS[] = {0.015 / 500, 0.04 / 500, 0.08 / 500, 0.2 / 500};

float textureShadow3x3(sampler2DArrayShadow tex, vec4 coord, vec2 sizeTex)
{
	coord.w -= BIAS[int(coord.z+0.5)];
	float res = texture(tex, coord);
	const float o_clamp = 1.5f/sizeTex.x;
	const float o = 1.5f/sizeTex.y;
	
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

