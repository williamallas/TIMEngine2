
layout(local_size_x=8, local_size_y=8, local_size_z=1) in;

struct Ray
{
	vec4 pos; // xyz, w:attenuation
	vec3 dir; // xyz
	int id_obj;
};
#ifdef LAST_PASS

#ifdef REFLEXION_PASS
layout(std140, binding = 0) coherent buffer reflectedRays { Ray reflected_rays[]; };
#endif
#ifdef REFRACTION_PASS
layout(std140, binding = 0) coherent buffer refractedRays { Ray refracted_rays[]; };
#endif

#else
layout(std140, binding = 0) coherent buffer reflectedRays { Ray reflected_rays[]; };
layout(std140, binding = 1) coherent buffer refractedRays { Ray refracted_rays[]; };
#endif


struct Shape
{
	uint type; // >= 1000 raymarched
	uvec3 packedData; // color, material, material2
	ivec4 textures; 
	vec4 shapeData;
	vec4 shapeData2;
};
Shape objects[7];
const int nbObjects = 7;
const int nbObjectsRc=6;
/*layout(std140, binding = 0) buffer all_objects { Shape objects[]; };*/

layout(binding = 0) uniform coherent writeonly image2D image0;

uniform sampler2D texture0;

uniform vec3 cameraWorld;
uniform vec3 cameraDir; 
uniform float time;

#define MAX_LIGHT 5
/*uniform */vec4 lightDirection[MAX_LIGHT];
/*uniform */vec4 lightDiffuse[MAX_LIGHT];
/*uniform */vec4 lightSpecular[MAX_LIGHT];
/*uniform */int nbLight;
/*uniform */vec4 globalAmbient;

uniform vec3 worldOrigin;
uniform mat4 worldOriginInvProjView;

#define ZERO_F 0.001

struct HitInfo
{
	uvec3 packedMat;
	ivec4 tex;
	vec3 tex_coord;
	int ID;
};

// root finding
float planeIntersection(in vec4 plane, in vec3 E_ray, in vec3 D_ray, out vec3 point, out vec3 normal);
float sphereIntersection(in vec4 sphere, in vec3 E_ray, in vec3 D_ray, out vec3 point, out vec3 normal);
float groundIntersection(in vec4 ground, in vec3 E_ray, in vec3 D_ray, out vec3 point, out vec3 normal);
float collide (in uint type, in vec4 shapeData, in vec3 E_ray, in vec3 D_ray, out vec3 point, out vec3 normal);
float collideWorld(in vec3 E, in vec3 D, out HitInfo hit, out vec3 point, out vec3 normalPoint);
float collideWorld_ignore(in vec3 E, in vec3 D, in int idIgnore, out HitInfo hit, out vec3 point, out vec3 normalPoint);

// raymarching
#define delta 0.005
#define NO_INTERSECTION 10000
float distance_world(in vec3 p);
float shadow_ray_march(in vec3 E, in vec3 D);
float distance_world(in vec3 p, out HitInfo hit);
float selective_ray_march(in vec3 E, in vec3 D, in float curT, out HitInfo hit);
float outside_ray_march(in vec3 E, in vec3 D);
float selective_ray_march_ignore(in vec3 E, in vec3 D, in int idIgnore, in float curT, out HitInfo hit);

float sss(in vec3 E, in vec3 D, in float range);
vec3 refractRay(vec3 D, vec3 N, float n1, float n2);

vec3 computeNormal(vec3 p);

vec4 applyLight(in vec3 p, in vec3 n, in HitInfo hit, in float sssfactor);

const vec4 bitSh = vec4(1.0, 255.0, 65025.0, 160581375.0);
const vec4 bitMsk = vec4(vec3(1./255.0),0);
const vec4 bitShifts = vec4(1.) / bitSh;
uint pack (vec4 v) {
	v *= 255;
    uint x=0;
	x += uint(v.r);
	x += uint(v.g)<<8;
	x += uint(v.b)<<16;
	x += uint(v.a)<<24;
	return x;
}
vec4 unpack (uint x) {
    vec4 res;
	res.x = x & 0xff;
	x=x>>8; res.y = x & 0xff;
	x=x>>8; res.z = x & 0xff;
	x=x>>8; res.w = x & 0xff;
	return res / 255.f;
}

#define RC_PLANE 0
#define RC_SPHERE 1
#define RC_GROUND 2
#define RM_PLANE 3
#define RM_SPHERE 4
#define RM_BOX 5
#define LAST_RC 2

void main()
{
	
#ifdef FIRST_PASS
	#define DEPTH_F -1
	vec3 tl = vec3(worldOriginInvProjView*vec4(-1,1,DEPTH_F,1));
    vec3 bl = vec3(worldOriginInvProjView*vec4(-1,-1,DEPTH_F,1));
    vec3 br = vec3(worldOriginInvProjView*vec4(1,-1,DEPTH_F,1));
	
	vec3 step_x = (br-bl) / (gl_NumWorkGroups.x * 8);
	vec3 step_y = (tl-bl) / (gl_NumWorkGroups.y * 8);
	
	vec3 pts = bl + step_x * gl_GlobalInvocationID.x + step_x*0.5 + 
		            step_y * gl_GlobalInvocationID.y + step_y*0.5;
					
	vec3 D = normalize(pts - cameraWorld - worldOrigin);
	vec3 E = cameraWorld - worldOrigin;
#endif
#ifdef REFLEXION_PASS
	uint globalIndex = gl_GlobalInvocationID.y*gl_NumWorkGroups.x*gl_WorkGroupSize.x + gl_GlobalInvocationID.x;
	Ray curRay = reflected_rays[globalIndex];
	if(curRay.pos.w == 0)
	{
		#ifndef LAST_PASS
		refracted_rays[globalIndex] = Ray(vec4(0), vec3(0),-1);
		#endif
		return;
	}
	vec3 D = curRay.dir;
	vec3 E = curRay.pos.xyz + D*delta*2;
#endif

#ifdef REFRACTION_PASS
	uint globalIndex = gl_GlobalInvocationID.y*gl_NumWorkGroups.x*gl_WorkGroupSize.x + gl_GlobalInvocationID.x;
	Ray curRay = refracted_rays[globalIndex];
	if(curRay.pos.w == 0)
	{
		#ifndef LAST_PASS
		reflected_rays[globalIndex] = Ray(vec4(0), vec3(0),-1);
		#endif
		return;
	}
	vec3 D = curRay.dir;
	vec3 E = curRay.pos.xyz;
#endif


	lightDirection[0] = vec4(1,1,-1,0);
	lightDiffuse[0] = vec4(1,1,1,1);
	lightSpecular[0] = vec4(1,1,1,1);
	nbLight=1;
	globalAmbient = vec4(0.1);
		
	/*objects[0] = Shape(RC_PLANE, uvec3( pack(vec4(1,1,1,0.2)), pack(vec4(1,1,0.3,0)), pack(vec4(0,0,0,0))),
		ivec4(1,0,0,0), vec4(0,0,1,0),vec4(0,0,0,0)); // plane*/
		
	objects[0] = Shape(RC_GROUND, uvec3( pack(vec4(1,1,1,0.2)), pack(vec4(1,0.5,1,0)), pack(vec4(0,0.5,0,0))),
		ivec4(1,0,0,0), vec4(0,0,0,10),vec4(0,0,0,0)); 
	
	objects[1] = Shape(RC_SPHERE, uvec3( pack(vec4(0.8,0.3,0.3,0.8)), pack(vec4(1,1,1,0)), pack(vec4(0,0,0,0.05))),
		ivec4(0), vec4(-1,-1,1,1), vec4(0)); // sphere
		
	objects[2] = Shape(RC_SPHERE, uvec3( pack(vec4(0.3,0.8,0.3,0.8)), pack(vec4(1,1,1,0)), pack(vec4(0,0,0,0.05))), 
		ivec4(0), vec4(1,-1,1,1), vec4(0)); // sphere
		
	objects[3] = Shape(RC_SPHERE, uvec3( pack(vec4(0.3,0.3,0.8,0.8)), pack(vec4(1,1,1,0)), pack(vec4(0,0,0,0.05))), 
		ivec4(0), vec4(0,1,1,1), vec4(0)); // sphere
		
	objects[4] = Shape(RC_SPHERE, uvec3( pack(vec4(1,1,1,1)), pack(vec4(1,0.2,1,0)), pack(vec4(0,0,0,0))), 
		ivec4(0), vec4(0,0,3,1), vec4(0)); // sphere
		
	objects[5] = Shape(RC_SPHERE, uvec3( pack(vec4(0.2,0.2,0.2,0.8)), pack(vec4(1,1,1,0)), pack(vec4(0,0,0.8,0.05))),
		ivec4(0), vec4(5,0,1,1), vec4(0)); 
		
	objects[6] = Shape(RM_BOX, uvec3( pack(vec4(0,0.33,0,0.2)), pack(vec4(1,1,1,0)), pack(vec4(0,0.5,0,0))), 
		ivec4(0), vec4(0,8,4.01,0), vec4(4,0.3,4,0)); // box
	
	
	vec3 nearestPoint=vec3(0), normalPoint=vec3(0);
	HitInfo hit1;
	#ifndef REFRACTION_PASS
	float t = collideWorld(E,D, hit1, nearestPoint, normalPoint);
	#else
	float t = collideWorld_ignore(E,D, curRay.id_obj, hit1, nearestPoint, normalPoint);
	#endif
	
	/*HitInfo hit2;
	#ifndef REFRACTION_PASS
	float t2 = selective_ray_march(E,D, t, hit2);
	#else
	float t2 = selective_ray_march_ignore(E,D, curRay.id_obj,t, hit2);
	#endif
	if(t > t2)
	{
		nearestPoint = E+t2*D;
		normalPoint = computeNormal(nearestPoint);
		hit1=hit2;
	}*/
	
	//if(min(t,t2) != NO_INTERSECTION)
	if(t != NO_INTERSECTION)
	{
		vec4 mat2 = unpack(hit1.packedMat.z);
		vec4 curColor = applyLight(nearestPoint, normalPoint, hit1, /*mat2.x>0?sss(nearestPoint, D, mat2.x):*/0);
		#if defined(REFLEXION_PASS) || defined(REFRACTION_PASS)
		curColor *= curRay.pos.w;
		curColor += texelFetch(texture0, ivec2(gl_GlobalInvocationID.xy),0);
		#endif
		imageStore(image0, ivec2(gl_GlobalInvocationID.xy), curColor);
		
		/*#ifndef LAST_PASS
		vec3 reflectedVec = reflect(D, normalPoint);
		uint globalIndex = gl_GlobalInvocationID.y*gl_NumWorkGroups.x*gl_WorkGroupSize.x + gl_GlobalInvocationID.x;
		reflected_rays[globalIndex] = Ray(vec4(nearestPoint, mat2.y),vec3(reflectedVec), hit1.ID);
		
		if(mat2.z > 0)
		{
			vec3 refractedRay = normalize(refractRay(D, normalPoint, 1, 1+mat2.w));
			refracted_rays[globalIndex] = Ray(vec4(nearestPoint, mat2.z),vec3(refractedRay), hit1.ID);
		}
		else refracted_rays[globalIndex] = Ray(vec4(0),vec3(0),-1);
		
		#endif*/
	}
	else
	{
		vec4 curColor = vec4(0,0,1,1);
		#if defined(REFLEXION_PASS) || defined(REFRACTION_PASS)
		curColor *= curRay.pos.w;
		curColor += texelFetch(texture0, ivec2(gl_GlobalInvocationID.xy),0);
		#endif
		imageStore(image0, ivec2(gl_GlobalInvocationID.xy), curColor);
		
		#ifndef LAST_PASS
		uint globalIndex = gl_GlobalInvocationID.y*gl_NumWorkGroups.x*gl_WorkGroupSize.x + gl_GlobalInvocationID.x;
		reflected_rays[globalIndex] = Ray(vec4(0), vec3(0),-1);
		refracted_rays[globalIndex] = Ray(vec4(0), vec3(0),-1);
		#endif
	}
	
	barrier();
}

float collideWorld(in vec3 E, in vec3 D, out HitInfo hit, out vec3 point, out vec3 normalPoint)
{
	float t_cur = NO_INTERSECTION;
	
	uvec3 packedDat;
	ivec4 tex;
	vec3 tex_coord;
	int ID;
	
	for(int i=0 ; i<nbObjectsRc ; ++i)
	{
		uint type = objects[i].type;
		HitInfo inf = HitInfo(objects[i].packedData, objects[i].textures, vec3(0), 0);

		//if(type <= LAST_RC)
		//{
			vec3 p,n;
			float t = collide(type, objects[i].shapeData, E, D, p,n);
			
			int test = int(/*t > 0 && */t < t_cur);
			t_cur = (1-test)*t_cur + test*t;
			normalPoint = (1-test)*normalPoint+ test*n;
			point = (1-test)*point+ test*p;
			
			packedDat = (1-test)*packedDat + test*inf.packedMat;
			tex = (1-test)*tex + test*inf.tex;
			tex_coord = (1-test)*tex_coord + test*inf.tex_coord;
			ID = (1-test)*ID + test*i;
			
			hit = HitInfo(packedDat, tex, tex_coord, ID);
		//}
	}
	
	return t_cur;
}

float collideWorld_ignore(in vec3 E, in vec3 D, in int idIgnore, out HitInfo hit, out vec3 point, out vec3 normalPoint)
{
	float t_cur = NO_INTERSECTION;
	
	uvec3 packedDat;
	ivec4 tex;
	vec3 tex_coord;
	int ID;
	
	for(int i=0 ; i<nbObjects ; ++i)
	{
		uint type = objects[i].type;
		HitInfo inf = HitInfo(objects[i].packedData, objects[i].textures, vec3(0), 0);

		//if(type <= LAST_RC)
		//{
			vec3 p,n;
			float t = collide(type, objects[i].shapeData, E, D, p,n);
			
			int test = int(/*t > 0 && */t < t_cur && i != idIgnore);
			t_cur = (1-test)*t_cur + test*t;
			normalPoint = (1-test)*normalPoint+ test*n;
			point = (1-test)*point+ test*p;
			
			packedDat = (1-test)*packedDat + test*inf.packedMat;
			tex = (1-test)*tex + test*inf.tex;
			tex_coord = (1-test)*tex_coord + test*inf.tex_coord;
			ID = (1-test)*ID + test*i;
			
			hit = HitInfo(packedDat, tex, tex_coord, ID);
		//}
	}
	
	return t_cur;
}

float collide (in uint type, in vec4 shapeData, in vec3 E_ray, in vec3 D_ray, out vec3 point, out vec3 normal)
{
	switch(type)
	{
		case RC_PLANE: return planeIntersection(shapeData, E_ray, D_ray, point, normal);
		case RC_SPHERE: return sphereIntersection(shapeData, E_ray, D_ray, point, normal);
		case RC_GROUND: return groundIntersection(shapeData, E_ray, D_ray, point, normal);
		default: return NO_INTERSECTION;
	}
}

float planeIntersection(in vec4 plane, in vec3 E_ray, in vec3 D_ray, out vec3 point, out vec3 normal)
{
	float d = dot(plane.xyz, D_ray);
	if(d > -ZERO_F) return NO_INTERSECTION;
	float t = (plane.w - dot(plane.xyz, E_ray)) / d;
	point = E_ray + t*D_ray;
	normal = normalize(plane.xyz);
	return t;
}

float groundIntersection(in vec4 ground, in vec3 E_ray, in vec3 D_ray, out vec3 point, out vec3 normal)
{
	float d = dot(vec3(0,0,1), D_ray);
	if(d > -ZERO_F) return NO_INTERSECTION;
	float t = (0 - dot(vec3(0,0,1), E_ray)) / d;
	point = E_ray + t*D_ray;
	normal = vec3(0,0,1);
	
	int test = int(point.x > ground.w || point.x < -ground.w || point.y > ground.w || point.y < -ground.w);
	
	return (1-test)*t + test*NO_INTERSECTION;
}

float sphereIntersection(in vec4 sphere, in vec3 E_ray, vec3 D_ray, out vec3 point, out vec3 normal)
{
	E_ray = (E_ray - sphere.xyz);
	
	float a = dot(D_ray,D_ray);
	float b = 2*dot(E_ray, D_ray);
	float c = dot(E_ray,E_ray)-1;
	
	float det = b*b - 4*a*c;
	
	if(det < 0) return NO_INTERSECTION;
	else
	{
		float sqrt_det = sqrt(det);
		float t1 = (-b + sqrt_det) / 2*a;
		float t2 = (-b - sqrt_det) / 2*a;
		
		float min_t = min(t1,t2);
		float max_t = max(t1,t2);
		float t;
		/*if(t1<0 && t2<0) return NO_INTERSECTION;
		float t=min(t1,t2);*/
		
		if(min_t > 0)
			t=min_t;
		else if(max_t > 0)
			t=max_t;
		else return NO_INTERSECTION;
		
		point = (E_ray + t*D_ray);
		normal = normalize(point);
		
		point += sphere.xyz;
		
		return t;
	}
}

vec4 getTexture(in HitInfo hit,in vec3 p, in vec3 n)
{
	switch(hit.tex.x)
	{
		case 0: return vec4(1);
		case 1: 
			vec2 v=fract(p.xy);
			return vec4(float(int((v.x<0.5 && v.y<0.5) || (v.x>0.5 && v.y>0.5))));
		default: return vec4(1);
	}
}

vec4 applyLight(in vec3 p, in vec3 n, in HitInfo hit, in float sssfactor)
{
	vec4 final = vec4(0);
	vec4 color = unpack(hit.packedMat.x);
	vec4 material = unpack(hit.packedMat.y);
	vec4 texColor = getTexture(hit, p, n);

	
	for(int i=0 ; i<1 ; ++i)
	{
		vec3 lightDir = normalize(-lightDirection[i].xyz);
		float shadowCoef = shadow_ray_march(p, lightDir);
		
		float lightCoef = clamp(dot(n, lightDir),0,1) * shadowCoef;
		//final += lightCoef*vec4(1,1,1,1);
		final += lightDiffuse[i] * /*color * material.y **/ lightCoef * texColor;
		
		vec3 R = reflect(-lightDir, n);
		float shininess = color.w * 128;
		vec3 vertexDir = normalize(cameraWorld-worldOrigin-p);
		lightCoef = pow(max(dot(R, vertexDir),0), shininess) * shadowCoef;
		//final += lightSpecular[i] * color * material.z * lightCoef * texColor;
		
		//final += lightDiffuse[i] * color * material.y * clamp(lightCoef,0.3,1.f) * sssfactor * 1.5;
	}
	
	//final += material.x*color*globalAmbient*texColor;
	
	//final += material.x*color*globalAmbient*texColor;
	//final += material.w*5*color*texColor;
	
	return final;
}

float sphereDistance(vec3 p, vec4 sphere)
{
	p -= sphere.xyz;
	return sqrt(p.x*p.x + p.y*p.y + p.z*p.z) - sphere.w;
}

float planeDistance(vec3 p, vec4 plane)
{
	return dot(p, plane.xyz) + plane.w;
}

float boxDistance(in vec3 p, in vec3 box_pos, in vec3 b)
{
	p -= box_pos;
	vec3 d = abs(p) - b;
	return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float distance_world(in vec3 p, out HitInfo hit)
{
	float minDist = NO_INTERSECTION;
	
	for(int i=nbObjectsRc ; i<nbObjects ; ++i)
	{
		float dist = NO_INTERSECTION;
		switch(objects[i].type)
		{	
			case RM_BOX: dist = boxDistance(p, objects[i].shapeData.xyz, objects[i].shapeData2.xyz); break;
			case RM_PLANE: dist = planeDistance(p, objects[i].shapeData); break;
			case RM_SPHERE: dist = sphereDistance(p, objects[i].shapeData); break;
			default: break;
		}
		
		minDist = min(minDist,dist);
		int test = int(minDist == dist);
		hit.packedMat = hit.packedMat * (1-test) + objects[i].packedData * test;
		hit.tex = hit.tex * (1-test) + objects[i].textures * test;
		hit.ID = hit.ID * (1-test) + i*test;
	}
	
	return minDist;
}

float distance_world(in vec3 p, out HitInfo hit, in int idIgnore)
{
	float minDist = NO_INTERSECTION;
	
	for(int i=nbObjectsRc ; i<nbObjects ; ++i)
	{
		float dist = NO_INTERSECTION;
		switch(objects[i].type)
		{	
			case RM_BOX: dist = boxDistance(p, objects[i].shapeData.xyz, objects[i].shapeData2.xyz); break;
			case RM_PLANE: dist = planeDistance(p, objects[i].shapeData); break;
			case RM_SPHERE: dist = sphereDistance(p, objects[i].shapeData); break;
			default: break;
		}
		
		minDist = int(idIgnore != i)*min(minDist,dist) + (1-int(idIgnore != i))*minDist;
		
		int test = int(minDist == dist);
		hit.packedMat = hit.packedMat * (1-test) + objects[i].packedData * test;
		hit.tex = hit.tex * (1-test) + objects[i].textures * test;
		hit.ID = hit.ID * (1-test) + i*test;
	}
	
	return minDist;
}

float distance_world(in vec3 p)
{
	float minDist = NO_INTERSECTION;
	
	for(int i=0 ; i<nbObjects ; ++i)
	{
		float dist;
		switch(objects[i].type)
		{
			case RC_PLANE: case RM_PLANE: dist = planeDistance(p, objects[i].shapeData); break;
			case RC_SPHERE:case RM_SPHERE: dist = sphereDistance(p, objects[i].shapeData); break;
			case RM_BOX: dist = boxDistance(p, objects[i].shapeData.xyz, objects[i].shapeData2.xyz); break;
			default: dist=NO_INTERSECTION; break;
		}
		
		minDist = min(minDist,dist);
	}
	
	return minDist;
}

float distance_world_shadow(in vec3 p)
{
	float minDist = NO_INTERSECTION;
	
	for(int i=0 ; i<nbObjects ; ++i)
	{
		float dist;
		vec4 mat2 = unpack(objects[i].packedData.z);
		switch(objects[i].type)
		{
			case RC_PLANE: case RM_PLANE: dist = planeDistance(p, objects[i].shapeData); break;
			case RC_SPHERE:case RM_SPHERE: dist = sphereDistance(p, objects[i].shapeData); break;
			case RM_BOX: dist = boxDistance(p, objects[i].shapeData.xyz, objects[i].shapeData2.xyz); break;
			default: dist=NO_INTERSECTION; break;
		}
		
		minDist = int(mat2.z<0.75)*min(minDist,dist) + (1-int(mat2.z<0.75))*minDist;
	}
	
	return minDist;
}

float shadow_ray_march(in vec3 E, in vec3 D)
{
	float t=0.2;
	float d=0;
	float shadow=1;
	while(t < 50)
	{
		d = distance_world_shadow(E + t*D);
		shadow = min(shadow, 10 * d/t);
		if(d < 0) return 0;
		t += max(d,0.2);
	}
	return shadow;
}

float selective_ray_march(in vec3 E, in vec3 D, in float curT, out HitInfo hit)
{
	float t=0;
	float d=0;
	curT = min(curT, 100);
	while(t < curT)
	{
		d = distance_world(E + t*D, hit);
		if(d < delta) return t;
		t += d;
	}
	return NO_INTERSECTION;
}

float selective_ray_march_ignore(in vec3 E, in vec3 D, in int idIgnore, in float curT, out HitInfo hit)
{
	float t=0;
	float d=0;
	curT = min(curT, 100);
	while(t < curT)
	{
		d = distance_world(E + t*D, hit, idIgnore);
		if(d < delta) return t;
		t += d;
	}
	return NO_INTERSECTION;
}

vec3 computeNormal(vec3 p)
{
	vec3 n;
	n.x = distance_world(vec3(p.x + delta, p.yz)) - distance_world(vec3(p.x - delta, p.yz));
	n.y = distance_world(vec3(p.x, p.y + delta, p.z)) - distance_world(vec3(p.x, p.y - delta, p.z));
	n.z = distance_world(vec3(p.xy, p.z + delta)) - distance_world(vec3(p.xy, p.z - delta));
	return normalize(n);
}

float sss(in vec3 E, in vec3 D, in float range)
{
	#define NB_LOOKUP 4
	const float FF = NB_LOOKUP*(NB_LOOKUP+1)*0.5/NB_LOOKUP;
	float COEF[NB_LOOKUP];
	for(int i=0 ; i<NB_LOOKUP ; ++i)
		COEF[i] = float(i+1)/NB_LOOKUP;
	
	float d;
	for(int i=0 ; i<NB_LOOKUP ; ++i)
		d += distance_world(E + range*COEF[i]*D);
	
	d = clamp(d+range*FF, 0.f,range*FF*2) / (range*FF*2);
	return d;
}

vec3 refractRay(vec3 D, vec3 N, float n1, float n2)
{
	float n=n1/n2;
	float cos_i = dot(N,-D);
	float sin_t2 = n*n*(1-cos_i*cos_i);
	return n*D + (n*cos_i - sqrt(1-sin_t2))*N;
}