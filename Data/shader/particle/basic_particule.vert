#version 330
#define INSTANCING 128

invariant gl_Position;
in vec3 vertex;
in vec3 normal;
in vec2 texCoord;
in vec3 tangent;

uniform mat4 projView;
uniform mat4 view;
uniform mat4 model[INSTANCING];

flat out vec4 vertexWorld;
flat out vec4 vertexCamera;
flat out vec4 vertexScreen;
flat out vec3 pColor;
flat out vec2 pSize;
flat out vec2 alpha_rotation;

 void main()
 {
	pColor = normal;
	pSize = tangent.xy;
	alpha_rotation = texCoord;
 
	vertexWorld = model[gl_InstanceID]*vec4(vertex,1);
	gl_Position = projView*vertexWorld;
	vertexScreen = gl_Position;
	vertexCamera = view * vertexWorld;
 }