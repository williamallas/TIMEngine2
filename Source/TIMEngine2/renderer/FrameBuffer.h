#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "GLState.h"
#include "Texture.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    class FrameBuffer : boost::noncopyable
    {
    public:
        const static uint MAX_COLOR_ATTACHMENT = 8;
        static void setupDefferedFBO(FrameBuffer&, vector<Texture*>&);

        FrameBuffer();
        FrameBuffer(const uivec2&);
        FrameBuffer(const std::vector<Texture*>&, Texture* depth = nullptr);
        virtual ~FrameBuffer();

        void setResolution(const uivec2&);
        const uivec2& resolution() const;

        void attachTexture(uint, Texture*, uint level=0, uint layer=0);
        void attachDepthTexture(Texture*, uint layer=0);

        void bind() const;
        void unbind() const;

        void enableAllAttachment() const;

    private: 
        uint _id=0;
        uivec2 _resolution;
        bool _isTexAttached[MAX_COLOR_ATTACHMENT];
    };

    inline void FrameBuffer::setResolution(const uivec2& v) { _resolution = v; }
    inline const uivec2& FrameBuffer::resolution() const { return _resolution; }

}
}
#include "MemoryLoggerOff.h"

#endif // FRAMEBUFFER_H
