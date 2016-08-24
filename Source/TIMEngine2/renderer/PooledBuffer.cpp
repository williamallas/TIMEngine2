#include "PooledBuffer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

PooledBuffer::PooledBuffer(TextureBufferPool* pool) : _pool(pool)
{

}

void PooledBuffer::setParameter(const TextureBufferPool::Key& k)
{
    _param = k;
}

Texture* PooledBuffer::buffer(int index) const
{
    if(_counter == 0)
        return nullptr;

    return _buffer.texs[index];
}

FrameBuffer* PooledBuffer::fbo() const
{
    if(_counter == 0)
        return nullptr;

    return _buffer.fbo;
}

void PooledBuffer::acquire()
{
    if(!_pool)
        return;

    _counter++;
    if(_counter > 1)
        return;

    _buffer = _pool->getBuffer(_param);
}

void PooledBuffer::release()
{
    if(!_pool)
        return;
    _counter--;
    if(_counter < 0)
    {
        LOG("PooledBuffer::release() : counter<0");
        return;
    }

    if(_counter == 0)
    {
        _pool->releaseBuffer(_buffer.id);
    }
}

}
}

