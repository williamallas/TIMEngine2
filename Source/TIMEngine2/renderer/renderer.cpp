#include "renderer.h"

#include "GL/glew.h"
#include "Logger.h"
#include "Texture.h"
#include "ShaderCompiler.h"
#include "MeshBuffers.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

uint textureSampler[static_cast<uint>(TextureMode::Last)];

ThreadPool globalThreadPool(4);

void __stdcall debugOut(GLenum , GLenum , GLuint , GLenum severity , GLsizei , const GLchar* msg, GLvoid*)
{
    if(severity == GL_DEBUG_SEVERITY_HIGH)
    {
        LOG("OpenGL debug: ", msg);
    }
}

bool hasBeenInit = false;

bool init()
{
    GLenum glewerr;
    if((glewerr=glewInit()) != GLEW_OK)
    {
        LOG("glewinit() failed : ",glewGetErrorString(glewerr),"\n");
        return false;
    }
    glGetError();

    std::cout << "Initializing on "<<glGetString(GL_VENDOR)<<" "<<glGetString(GL_RENDERER)<<" using OpenGL "<<glGetString(GL_VERSION)<<"\n";
    LOG("Initializing on ",glGetString(GL_VENDOR)," ",glGetString(GL_RENDERER)," using OpenGL ",glGetString(GL_VERSION),"\n");

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback( debugOut, NULL );

    glClearDepth(1);
    openGL.clearDepth();
    openGL.getHardwardProperties();
    openGL.applyAll();

    LOG("\nSupport of bindless_texture: ",glewGetExtension("GL_ARB_bindless_texture")==GL_TRUE);
    LOG("Support of sparse_texture: ",glewGetExtension("GL_ARB_sparse_texture")==GL_TRUE);

    vertexBufferPool = new VertexBufferPoolType(2 << 20, VertexFormat::VNCT, DrawMode::DYNAMIC);
    indexBufferPool = new IndexBufferPoolType(8 << 20, DrawMode::DYNAMIC);
    indexBufferPool->buffer().bind();

    textureSampler[TextureMode::NoFilter] = Texture::genTextureSampler(false,false,false,false);
    textureSampler[TextureMode::Filtered] = Texture::genTextureSampler(true,true,true,false);
    textureSampler[TextureMode::FilteredNoRepeat] = Texture::genTextureSampler(false,true,true,false);
    textureSampler[TextureMode::DepthMap] = Texture::genTextureSampler(false,true,false,true);

    ShaderCompiler vDrawQuad(ShaderCompiler::VERTEX_SHADER);
    vDrawQuad.setSource(drawQuad_vertex);
    ShaderCompiler pDrawQuad(ShaderCompiler::PIXEL_SHADER);
    pDrawQuad.setSource(drawQuad_pixel);

    auto optShader = Shader::combine(vDrawQuad.compile({}), pDrawQuad.compile({}));
    if(!optShader.hasValue())
    {
        LOG_EXT("DrawQuad Vertex shader error:\n", vDrawQuad.error());
        LOG_EXT("DrawQuad Pixel shader error:\n", pDrawQuad.error());
        LOG_EXT("DrawQuad Link erorr:", Shader::lastLinkError());
    }
    else
        drawQuadShader = optShader.value();

    ShaderCompiler vDepthPass(ShaderCompiler::VERTEX_SHADER);
    vDepthPass.setSource(depthPass_vertex);
    LOG("No gs for DepthPass shader, expect a warning");
    optShader = Shader::linkVertexShader(vDepthPass.compile({}));
    if(!optShader.hasValue())
    {
        LOG_EXT("DepthPass Vertex shader error:\n", vDepthPass.error());
        LOG_EXT("DepthPass Link erorr:", Shader::lastLinkError());
    }
    else
        depthPassShader = optShader.value();

    VNCT_Vertex vData[4] = {{vec3(-1,-1,0),vec3(),vec2(0,0),vec3()},
                            {vec3(-1, 1,0),vec3(),vec2(0,1),vec3()},
                            {vec3( 1, 1,0),vec3(),vec2(1,1),vec3()},
                            {vec3( 1,-1,0),vec3(),vec2(1,0),vec3()}};
    uint iData[6] = {0,1,2,2,3,0};

    VBuffer* tmpVB = vertexBufferPool->alloc(4);
    IBuffer* tmpIB = indexBufferPool->alloc(6);
    tmpVB->flush(reinterpret_cast<float*>(vData),0,4);
    tmpIB->flush(iData,0,6);
    quadMeshBuffers = new MeshBuffers(tmpVB,tmpIB);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    if(openGL.hardward(GLState::Hardward::MAJOR_VERSION) > 4 ||
       (openGL.hardward(GLState::Hardward::MAJOR_VERSION)==4 && openGL.hardward(GLState::Hardward::MINOR_VERSION)>=3))
    {
        hasBeenInit = true;
        return true;
    }
    else
    {
        LOG("You don't have a sufficient openGL version.");
        return false;
    }

}

bool close()
{
    if(!hasBeenInit) return true;

    hasBeenInit = false;

    for(int i=0 ; i<TextureMode::Last ; ++i)
        Texture::removeTextureSampler(textureSampler[i]);

    delete drawQuadShader;
    delete quadMeshBuffers;
    delete depthPassShader;
    delete vertexBufferPool;
    delete indexBufferPool;
    GLState::freeInstance();
    return true;
}

BufferPool<VertexBuffer, 64>* vertexBufferPool = nullptr;
BufferPool<IndexBuffer, 256>* indexBufferPool = nullptr;

const char* drawQuad_vertex =
    "#version 430 \n"
    "in vec3 vertex; \n"
    "in vec2 texCoord; \n"
    "smooth out vec2 coord; \n"
    "void main() { \n"
        "gl_Position = vec4(vertex,1); \n"
        "coord = texCoord; \n"
    "}";

const char* drawQuad_pixel =
    "#version 430 \n"
    "smooth in vec2 coord; \n"
    "out vec4 outColor0; \n"
    "uniform sampler2D texture0; \n"
    "void main() { \n"
        "outColor0 = texture(texture0, coord); \n"
    "}";

Shader* drawQuadShader = nullptr;
MeshBuffers* quadMeshBuffers = nullptr;

const char* depthPass_vertex = R"(
    #version 430

    in vec3 vertex;
    in int drawId;

    uniform mat4 projView;

    layout(std140, binding = 1) uniform ModelMatrix
    {
        mat4 models[MAX_UBO_VEC4 / 4];
    };

    void main() {
        gl_Position = projView * models[drawId] * vec4(vertex,1);
    })";

Shader* depthPassShader = nullptr;

}
}
