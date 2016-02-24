#ifndef LIGHTCONTEXTRENDERER_H_INCLUDED
#define LIGHTCONTEXTRENDERER_H_INCLUDED

#include "DeferredRenderer.h"
#include "FrameParameter.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

    class LightContextRenderer : public AbstractRenderer
    {
    public:
        struct Light
        {
            enum { POINT, SPOT };
            int type;
            float radius, power;
            vec3 position;
            vec4 color;
        };

        LightContextRenderer(const DeferredRenderer& gbuffers, bool hdr=false);
        virtual ~LightContextRenderer();

        const DeferredRenderer& deferred() const { return _deferred; }
        Texture* buffer() const { return _buffer; }

    protected:
        const DeferredRenderer& _deferred;
        Texture* _buffer;
    };


}
}
#include "MemoryLoggerOff.h"


#endif // LIGHTCONTEXTRENDERER_H_INCLUDED
