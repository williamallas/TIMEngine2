
#include"LightContextRenderer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{


LightContextRenderer::LightContextRenderer(const DeferredRenderer& gbuffers, bool hdr) : AbstractRenderer(gbuffers.resolution(), gbuffers.frameState()), _deferred(gbuffers)
{
    TextureBufferPool::Key k;
    k.type = TextureBufferPool::Key::NONE;
    k.res = uivec3(gbuffers.resolution(), 1);
    k.onlyTextures = false;
    k.hdr = hdr;

    _buffer.setParameter(k);
}

LightContextRenderer::~LightContextRenderer()
{

}

void LightContextRenderer::clear() const
{
    _buffer.fbo()->bind();
	openGL.clearColor(vec4::construct(0));
}

}
}


