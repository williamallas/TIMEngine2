#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include "VertexFormat.h"
#include "GLState.h"
#include "GpuBuffer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    class IndexBuffer : public GpuBuffer<uint, GpuBufferPolicy::ElementArrayBuffer>
    {
    public:
        IndexBuffer() = default;
        ~IndexBuffer() = default;

        using GpuBuffer<uint, GpuBufferPolicy::ElementArrayBuffer>::elementSize;
        using GpuBuffer<uint, GpuBufferPolicy::ElementArrayBuffer>::id;

        void bind() const { GpuBufferPolicy::ElementArrayBuffer::bind(id()); }

        void draw(size_t, VertexMode, size_t) const;

        static const uint GLPrimitive[6]; // see VertexMode

    private:
        #include "MemoryLoggerOff.h"
        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;
        #include "MemoryLoggerOn.h"
    };
}
}
#include "MemoryLoggerOff.h"

#endif // INDEXBUFFER_H
