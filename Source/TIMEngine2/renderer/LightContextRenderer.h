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
            vec3 direction;
            float cutoff;
        };

        LightContextRenderer(const DeferredRenderer& gbuffers, bool hdr=false);
        virtual ~LightContextRenderer();

        const DeferredRenderer& deferred() const { return _deferred; }
        Texture* buffer() const { return _buffer.buffer(0); }

		void clear() const;

    protected:
        const DeferredRenderer& _deferred;
    };


}
}
#include "MemoryLoggerOff.h"


#endif // LIGHTCONTEXTRENDERER_H_INCLUDED
