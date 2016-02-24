#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include "core/core.h"
#include "FrameBuffer.h"
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
        AbstractRenderer(const uivec2& res, const FrameParameter& param) : _frameState(param), _resolution(res), _fbo(res) {}
        virtual ~AbstractRenderer() = default;

        const uivec2& resolution() const { return _resolution; }
        const FrameBuffer& frameBuffer() const { return _fbo; }
        const FrameParameter& frameState() const { return _frameState; }

    protected:
        const FrameParameter& _frameState;
        uivec2 _resolution;
        FrameBuffer _fbo;
    };

}
}
#include "MemoryLoggerOff.h"

#endif // ABSTRACTRENDERER_H
