#include "DeferredRenderer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

DeferredRenderer::DeferredRenderer(const uivec2& res) : AbstractRenderer(res)
{
    FrameBuffer::setupDefferedFBO(_fbo, _buffers);
//    for(int i=0 ; i<4 ; ++i)
//        _interfaceBuffers[i] = new BufferSelector(buffer(i));
}

DeferredRenderer::~DeferredRenderer()
{
    for(size_t i=0 ; i<_buffers.size() ; ++i)
        delete _buffers[i];

//    for(int i=0 ; i<4 ; ++i)
//        delete _interfaceBuffers[i];
}


//void DeferredRenderer::draw(vector<MaterialInstance*>& materials)
//{
//    _fbo->bind();
//    renderer::openGL.clearColor(_backColor);
//    renderer::openGL.clearDepth();

//    drawList(materials, MaterialPass::COLOR_PASS);

//    _fbo->unbind();
//}

}
}
