#include "PostReflexionRenderer.h"
#include "ShaderCompiler.h"
#include "Texture.h"
#include "renderer.h"
#include "MeshBuffers.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

PostReflexionRenderer::PostReflexionRenderer(renderer::LightContextRenderer& l) : _context(l)
{
    ShaderCompiler vShader(ShaderCompiler::VERTEX_SHADER), pShader(ShaderCompiler::PIXEL_SHADER);
    vShader.setSource(StringUtils::readFile("shader/fs_reflexion.vert"));
    pShader.setSource(StringUtils::readFile("shader/fs_reflexion.frag"));

    auto optShader = Shader::combine(vShader.compile({}), pShader.compile({}));

    if(!optShader.hasValue())
    {
        LOG("shader/fs_reflexion.vert error:\n", vShader.error());
        LOG("shader/fs_reflexion.frag error:\n", pShader.error());
        LOG("Link erorr:", Shader::lastLinkError());
    }
    else
    {
        _reflexionShader = optShader.value();
    }

    _state.setBlend(true);
    _state.setBlendEquation(DrawState::BlendEquation::ADD);
    _state.setBlendFunc({DrawState::BlendFunc::ONE, DrawState::BlendFunc::ONE});
    _state.setCullFace(false);
    _state.setWriteDepth(false);
    _state.setDepthTest(false);
    _state.setShader(_reflexionShader);
}

PostReflexionRenderer::~PostReflexionRenderer()
{
    delete _reflexionShader;
}

void PostReflexionRenderer::draw()
{
//    if(!input.empty() && input[0])
//    {
//        if(input[0]->resolution() != _resolution)
//        {
//            _resolution = input[0]->resolution();

//            delete _output;
//            Texture::GenTexParam param;
//            param.nbLevels = 1;
//            param.size = uivec3(_resolution,0);
//            param.format = input[0]->format();

//            _output = Texture::genTexture2D(param);
//            _fbo.attachTexture(0, _output);
//            _fbo.setResolution(input[0]->resolution());
//            _fbo.unbind();
//        }
//    }
//    else return;

//    _fbo.bind();
//    _states.bind();

//    for(uint i=0 ; i<input.size() ; ++i)
//    {
//        if(input[i])
//        {
//            openGL.bindTexture(input[i]->id(), Texture::toGLType(input[i]->type()), i);
//            openGL.bindTextureSampler(textureSampler[TextureMode::NoFilter], i);
//        }
//    }

//    _frameState.bind(0);

//    quadMeshBuffers->draw(6, VertexMode::TRIANGLES, 1);


    _context.frameBuffer().bind();
    _state.bind();

    for(int i=0 ; i<4 ; ++i)
    {
        openGL.bindTextureSampler(textureSampler[TextureMode::NoFilter], i);
        _context.deferred().buffer(i)->bind(i);
    }

    _context.frameState().bind(0);
    quadMeshBuffers->draw(6, VertexMode::TRIANGLES, 1);
}

}
}
