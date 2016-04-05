#include "DeferredRenderer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

DeferredRenderer::DeferredRenderer(const uivec2& res, const FrameParameter& param) : AbstractRenderer(res, param)
{
    FrameBuffer::setupDefferedFBO(_fbo, _buffers);
}

DeferredRenderer::~DeferredRenderer()
{
    for(size_t i=0 ; i<_buffers.size() ; ++i)
        delete _buffers[i];
}

}
}
