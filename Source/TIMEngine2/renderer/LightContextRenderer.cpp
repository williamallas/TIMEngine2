
#include"LightContextRenderer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{


LightContextRenderer::LightContextRenderer(const DeferredRenderer& gbuffers, bool hdr) : AbstractRenderer(gbuffers.resolution(), gbuffers.frameState()), _deferred(gbuffers)
{
    Texture::GenTexParam param;
    param.size = uivec3(gbuffers.resolution(),0);
    param.nbLevels = 1;
    param.format = hdr ? Texture::RGBA16F : Texture::RGBA8;

    _buffer = Texture::genTexture2D(param);

    _fbo.attachTexture(0, _buffer);
    _fbo.unbind();
}

LightContextRenderer::~LightContextRenderer()
{
    delete _buffer;
}

}
}


