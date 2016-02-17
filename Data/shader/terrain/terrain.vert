#version 330  
invariant gl_Position;  
#define INSTANCING 128  
in vec3 vertex;  
  
uniform mat4 projView;  
uniform mat4 model[INSTANCING];  
uniform sampler2D texture0;  
uniform sampler2D texture1;  
uniform vec4 userData[1]; // coordx coordy sizePatch zscale  
smooth out vec3 norm;  
smooth out vec3 vertexWorld;  
smooth out vec3 texCoord;  
  
vec4 texture_bilinear(sampler2D tex, vec2 uv)  
{  
    int textureSize = textureSize(texture0,0).x;  
    uv = clamp(uv, 0, 0.999999);  
    uv = uv*(textureSize-1);  
    ivec2 i_uv=ivec2(floor(uv.x), floor(uv.y));  
    uv = fract(uv);  
  
    vec4 tA = mix(texelFetch(tex, i_uv, 0),  
                   texelFetch(tex, i_uv + ivec2(1,0), 0),  
                   uv.x);  
    vec4 tB = mix(texelFetch(tex, i_uv + ivec2(0,1), 0),  
                   texelFetch(tex, i_uv + ivec2(1,1), 0),  
                   uv.x);  
  
    return mix(tA, tB, uv.y);  
}  
  
void main()  
{  
   vec3 vvertex=vec3(vertex.x, vertex.y,0); 
   vertexWorld=vec3(model[gl_InstanceID]*vec4(vvertex,1));  
   
   vec2 coord = ((vertexWorld.xy - userData[0].xy*userData[0].z + userData[0].z*0.5)/userData[0].z);  
   vertexWorld.z = texture_bilinear(texture0, coord).r;  
   
   texCoord = vec3(coord.x, coord.y, vertexWorld.z);  
   vertexWorld.z *= userData[0].w;  
   vertexWorld.z += vertex.z;  
   
   norm = normalize(texture_bilinear(texture1, coord.xy).xyz*2-1);  
   float f = norm.x; norm.x=norm.y; norm.y=f;  
   gl_Position=projView*vec4(vertexWorld,1);   
 }