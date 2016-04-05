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
        ~DeferredRenderer();

        Texture* buffer(uint) const;

    private:
        vector<Texture*> _buffers;

    };

    inline Texture* DeferredRenderer::buffer(size_t i) const
    {
        return _buffers[i];
    }
}
}
#include "MemoryLoggerOff.h"

#endif // DEFERREDRENDERER_H
