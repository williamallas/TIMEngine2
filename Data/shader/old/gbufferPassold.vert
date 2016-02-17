#version 330  
 #define INSTANCING 128  
 invariant gl_Position;  
 in vec3 vertex;  
 in vec3 normal;  
#ifdef NORMAL_MAPPING  
 #ifndef USE_TEXTURE  
    in vec2 texCoord;  
    out vec2 texCoord0;  
 #endif  
 in vec3 tangent;  
 smooth out vec3 in_tangent;  
#endif  
 smooth out vec3 in_normal;  
  
#ifdef USE_TEXTURE  
 in vec2 texCoord;  
 out vec2 texCoord0;  
#endif  
  
#ifdef DIRECTIONAL_LIGHT_TEST  
 smooth out vec3 vertexWorld;  
#endif  
  
#ifndef NO_MATRIX  
 uniform mat4 projView;  
 uniform mat4 model[INSTANCING];  
    #ifdef BONES_MATRIX  
     uniform mat4 bonesMatrix[24];  
     in vec4 auxVertexAttrib0;  
     in ivec4 auxVertexAttrib_i0;  
     //smooth out vec4 v_color; 
    #endif  
#endif  
#ifdef SIMPLE_MODEL  
    uniform mat4 model;  
#endif  
  
 void main()  
 {  
#ifndef NO_MATRIX  
    mat4 mat = projView*model[gl_InstanceID];  
    #ifdef BONES_MATRIX  
        gl_Position = auxVertexAttrib0.x * mat * bonesMatrix[auxVertexAttrib_i0.x] * vec4(vertex,1);  
        gl_Position +=     auxVertexAttrib0.y * mat * bonesMatrix[auxVertexAttrib_i0.y] * vec4(vertex,1);  
        gl_Position +=     auxVertexAttrib0.z * mat * bonesMatrix[auxVertexAttrib_i0.z] * vec4(vertex,1);  
        gl_Position +=     auxVertexAttrib0.w * mat * bonesMatrix[auxVertexAttrib_i0.w] * vec4(vertex,1);  
         
        //vec4 col[3]; col[0]=vec4(1,0,0,0); col[1]=vec4(0,1,0,0); col[2]=vec4(0,0,1,0);  
        //v_color = col[max(auxVertexAttrib_i0.x%3,0)]*auxVertexAttrib0.x + col[max(auxVertexAttrib_i0.y%3,0)]*auxVertexAttrib0.y + col[max(auxVertexAttrib_i0.z%3,0)]*auxVertexAttrib0.z;  
    #else  
        gl_Position=mat*vec4(vertex,1);  
    #endif  
#else  
    gl_Position=vec4(vertex,1);  
    #ifdef SIMPLE_MODEL  
        gl_Position=model*vec4(vertex,1);  
    #endif  
#endif  
  
#ifdef USE_TEXTURE  
    #ifndef NO_MATRIX  
        texCoord0 = texCoord;  
    #else  
        texCoord0 = vertex.xy*0.5+0.5;  
    #endif  
#endif  
#ifdef NORMAL_MAPPING  
 #ifndef USE_TEXTURE  
    texCoord0 = texCoord;  
 #endif  
#endif  
#ifndef NO_MATRIX  
        mat3 normalMat=mat3(model[gl_InstanceID]);  
    #ifdef ACCURATE_NORMAL  
        normalMat = transpose(inverse(normalMat));  
    #endif  
    #ifdef BONES_MATRIX  
        in_normal =      auxVertexAttrib0.x * normalMat * mat3(bonesMatrix[auxVertexAttrib_i0.x]) * normal;  
        in_normal +=     auxVertexAttrib0.y * normalMat * mat3(bonesMatrix[auxVertexAttrib_i0.y]) * normal;  
        in_normal +=     auxVertexAttrib0.z * normalMat * mat3(bonesMatrix[auxVertexAttrib_i0.z]) * normal;  
        in_normal +=     auxVertexAttrib0.w * normalMat * mat3(bonesMatrix[auxVertexAttrib_i0.w]) * normal;  
        in_normal = normalize(in_normal);  
        #ifdef NORMAL_MAPPING  
            in_tangent =      auxVertexAttrib0.x * normalMat * mat3(bonesMatrix[auxVertexAttrib_i0.x]) * tangent;  
            in_tangent +=     auxVertexAttrib0.y * normalMat * mat3(bonesMatrix[auxVertexAttrib_i0.y]) * tangent;  
            in_tangent +=     auxVertexAttrib0.z * normalMat * mat3(bonesMatrix[auxVertexAttrib_i0.z]) * tangent;  
            in_tangent +=     auxVertexAttrib0.w * normalMat * mat3(bonesMatrix[auxVertexAttrib_i0.w]) * tangent;  
            in_tangent = normalize(in_tangent);  
        #endif  
         
    #else  
        in_normal=normalize(normalMat*normal);  
        #ifdef NORMAL_MAPPING  
            in_tangent = normalize(normalMat*tangent);  
        #endif  
    #endif  
      
    #ifdef DIRECTIONAL_LIGHT_TEST  
        vertexWorld=vec3(model[gl_InstanceID]*vec4(vertex,1));  
    #endif  
#else  
    in_normal=normalize(normal);  
#endif  
 }