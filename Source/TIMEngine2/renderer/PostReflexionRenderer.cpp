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
//    _context.frameBuffer()->bind();
//    _state.bind();

//    for(int i=0 ; i<4 ; ++i)
//    {
//        _context.deferred().buffer(i)->bind(i);
//        openGL.bindTextureSampler(textureSampler[TextureMode::NoFilter], i);
//    }

//    _context.frameState().bind(0);
//    quadMeshBuffers->draw(6, VertexMode::TRIANGLES, 1);
}

}
}
