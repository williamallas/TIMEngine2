#ifndef POOLEDBUFFER_H
#define POOLEDBUFFER_H

#include "TextureBufferPool.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    class PooledBuffer : boost::noncopyable
    {
    public:
        PooledBuffer(TextureBufferPool*);

        void setParameter(const TextureBufferPool::Key&);

        Texture* buffer(int) const;
        FrameBuffer* fbo() const;

        void acquire();
        void release();

    protected:
        TextureBufferPool* _pool = nullptr;

        int _counter = 0;
        TextureBufferPool::Key _param;
        TextureBufferPool::Buffer _buffer;
    };
}
}
#include "MemoryLoggerOff.h"

#endif // POOLEDBUFFER_H
