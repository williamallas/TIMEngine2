#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include "core/core.h"
#include "renderer.h"
#include "PooledBuffer.h"
#include "FrameParameter.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

    class AbstractRenderer : boost::noncopyable
    {
    public:
        AbstractRenderer(const uivec2& res, const FrameParameter& param) : _frameState(param), _resolution(res), _buffer(renderer::texBufferPool) {}
        virtual ~AbstractRenderer() = default;

        const uivec2& resolution() const { return _resolution; }
        const FrameBuffer* frameBuffer() const { return _buffer.fbo(); }
        const FrameParameter& frameState() const { return _frameState; }

        void acquire() { _buffer.acquire(); }
        void release() { _buffer.release(); }

    protected:
        const FrameParameter& _frameState;
        uivec2 _resolution;
        PooledBuffer _buffer;
    };

}
}
#include "MemoryLoggerOff.h"

#endif // ABSTRACTRENDERER_H
