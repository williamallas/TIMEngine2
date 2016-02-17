#version 330
#define INSTANCING 128

 invariant gl_Position;
 in vec3 vertex;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 projView;
uniform mat4 model[INSTANCING];
uniform float time;
uniform vec3 cameraDir;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

const vec3 BOX = vec3(20,20,20);

flat out vec4 vertexWorld;
flat out vec4 vertexCamera;
flat out vec4 vertexScreen;

 void main()
 {
	vec3 base = (model[gl_InstanceID] * vec4(0,0,0,1)).xyz;
	base += cameraDir*length(BOX.x/2);
	
	vec3 vertexScaled = vertex * BOX; // pos of vertex in "base" coord
	vec3 v = (base - vertexScaled)/BOX;
	v = vec3(floor(v.x), floor(v.y), floor(v.z)) * BOX + vertexScaled + BOX*0.5; // final pos without any transformation
	
	v = (v - base) + BOX*0.5; // v is in base coord, in the positive part
	
	/* wathever */
	v += vec3(0,0,-time*4);
	/*		    */
	
	v = fract(v/BOX) * BOX;
	v = v - BOX*0.5 + base;
	
	gl_Position = projView*vec4(v,1);
	vertexScreen = gl_Position;
	vertexCamera = view * vec4(v,1);
	vertexWorld = vec4(v,1);
 }