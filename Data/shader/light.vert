#version 430

precision highp float;

invariant gl_Position;  
#define INSTANCING 128  
in vec3 vertex;  

 struct Light
{
	vec4 head; // (type, radius, x, x)
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation;
    vec4 spotData;
};
layout(std140, binding = 0) buffer lightsBuffer { Light lights[]; };
  
uniform mat4 projView;  
uniform mat4 model[INSTANCING];  
uniform int drawIndex;

flat out vec4 light_head; // (type, radius, x, x)
flat out vec4 light_diffuse;
flat out vec4 light_specular;
flat out vec4 light_attenuation;
flat out vec4 light_spotData;
flat out vec3 light_position;

void main()  
{   
   int index = gl_InstanceID + drawIndex;
   light_head = lights[index].head;
   light_diffuse = lights[index].diffuse;
   light_specular = lights[index].specular;
   light_attenuation = lights[index].attenuation;
   light_spotData = lights[index].spotData;
   
   gl_Position=projView*model[gl_InstanceID]*vec4(vertex*light_head.y,1);
   light_position = vec3(model[gl_InstanceID][3][0], model[gl_InstanceID][3][1], model[gl_InstanceID][3][2]);
 }