#version 330
 layout(points) in;
 layout(triangle_strip, max_vertices=4) out;
 
  float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

uniform mat4 projection;
uniform mat4 view;
uniform mat4 projView;
uniform vec3 cameraDir, cameraUp;

uniform float exponent; // size particule 

flat in vec4 vertexWorld[];
flat in vec4 vertexCamera[];
flat in vec4 vertexScreen[];

smooth out vec2 texCoord;

void emitScreenQuad(vec2 s);
void emitZWorld(vec2 s);

 void main()
 {
	//emitScreenQuad();
	emitZWorld(vec2(exponent, exponent*10));
 }  
 
 void emitScreenQuad(vec2 s)
 {
	vec4 projVertex = projection*vertexCamera[0];
	vec4 SIZE = projection * vec4(s,0,0);
	
	texCoord = vec2(0,0);
	gl_Position=projVertex + vec4(-SIZE.x,-SIZE.y,0,0);
	EmitVertex();
	
	texCoord = vec2(1,0);
	gl_Position=projVertex + vec4(SIZE.x,-SIZE.y,0,0);
	EmitVertex();
	
	texCoord = vec2(0,1);
	gl_Position=projVertex + vec4(-SIZE.x,SIZE.y,0,0);
	EmitVertex();
	
	texCoord = vec2(1,1);
	gl_Position=projVertex + vec4(SIZE.x,SIZE.y,0,0);
	EmitVertex();
	
	EndPrimitive();
 }
 
 void emitZWorld(vec2 s)
 {
	vec3 X = normalize(cross(cameraDir, cameraUp));
	
	texCoord = vec2(0,0);
	gl_Position = vec4(-X*s.x - vec3(0,0,1)*s.y,0) + vertexWorld[0];
	gl_Position = projView * gl_Position;
	EmitVertex();
	
	texCoord = vec2(0,1);
	gl_Position = vec4(-X*s.x + vec3(0,0,1)*s.y,0) + vertexWorld[0];
	gl_Position = projView * gl_Position;
	EmitVertex();
	
	texCoord = vec2(1,0);
	gl_Position = vec4(+X*s.x - vec3(0,0,1)*s.y,0) + vertexWorld[0];
	gl_Position = projView * gl_Position;
	EmitVertex();
	
	texCoord = vec2(1,1);
	gl_Position = vec4(+X*s.x + vec3(0,0,1)*s.y,0) + vertexWorld[0];
	gl_Position = projView * gl_Position;
	EmitVertex();
	
	EndPrimitive();
 }