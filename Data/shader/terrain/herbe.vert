#version 330
#define INSTANCING 128
 invariant gl_Position;
 in vec3 vertex;
 in vec3 normal;
 flat out vec3 t_normal;
 flat out mat4 modelMat;

#ifndef NO_MATRIX
 uniform mat4 projView;
 uniform mat4 model[INSTANCING];
#endif

 void main()
 {
    gl_Position=vec4(vertex,1);
	modelMat = model[gl_InstanceID];
	
    mat3 normalMat=mat3(model[gl_InstanceID]);
	
#ifdef ACCURATE_NORMAL
	normalMat = transpose(inverse(normalMat));
#endif
	
    t_normal=normal;//normalize(normalMat*normal);
 }