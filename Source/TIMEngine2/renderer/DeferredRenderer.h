#ifndef DEFERREDRENDERER_H
#define DEFERREDRENDERER_H

#include "AbstractRenderer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

    class DeferredRenderer : public AbstractRenderer
    {
    public:
        DeferredRenderer(const uivec2&, const FrameParameter&);
        ~DeferredRenderer() = default;

        Texture* buffer(int) const;

    private:


    };

    inline Texture* DeferredRenderer::buffer(int i) const
    {
        return _buffer.buffer(i);
    }
}
}
#include "MemoryLoggerOff.h"

#endif // DEFERREDRENDERER_H
