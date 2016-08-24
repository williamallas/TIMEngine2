#include "DeferredRenderer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

DeferredRenderer::DeferredRenderer(const uivec2& res, const FrameParameter& param) : AbstractRenderer(res, param)
{
    TextureBufferPool::Key k;
    k.type = TextureBufferPool::Key::DEFERRED_BUFFER;
    k.res = {res[0], res[1], 1};
    k.onlyTextures = false;
    k.hdr = false;

    _buffer.setParameter(k);
}


}
}
