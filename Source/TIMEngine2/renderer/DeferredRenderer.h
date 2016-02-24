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
        virtual ~DeferredRenderer();

        Texture* buffer(uint) const;

//        void setBackColor(const vec4&);

//        void draw(vector<MaterialInstance*>&);

//        Texture* buffer(size_t);

        //SingleTargetRenderer* getBufferInterface(uint i) const { return _interfaceBuffers[i]; }

    private:
        vector<Texture*> _buffers;
       // vec4 _backColor;

//        class BufferSelector : public SingleTargetRenderer
//        {
//            public:
//                BufferSelector(Texture* buf) : _buf(buf) {}
//                virtual ~BufferSelector() = default;
//                Texture* buffer() const override { return _buf; }

//            private:
//                Texture* _buf;
//        };
//        SingleTargetRenderer* _interfaceBuffers[4];
    };

    inline Texture* DeferredRenderer::buffer(size_t i) const
    {
        return _buffers[i];
    }

//    inline void DeferredRenderer::setBackColor(const vec4& col) { _backColor = col; }

}
}
#include "MemoryLoggerOff.h"

#endif // DEFERREDRENDERER_H
