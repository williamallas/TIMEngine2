#ifndef IN_BUFFERRENDERER_NODE_H
#define IN_BUFFERRENDERER_NODE_H

#include "interface/Pipeline.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
namespace pipeline
{
    class InBufferRenderer : public Pipeline::TerminalNode
    {
    public:
        InBufferRenderer();
        ~InBufferRenderer() = default;

        renderer::FrameBuffer& fbo() { return _fbo; }
        const renderer::FrameBuffer& fbo() const { return _fbo; }

        void prepare() override;
        void render() override;

    private:
        renderer::DrawState _stateDrawQuad;
        renderer::FrameBuffer _fbo;
    };
}
}
}
#include "MemoryLoggerOff.h"

#endif // IN_BUFFERRENDERER_NODE_H
