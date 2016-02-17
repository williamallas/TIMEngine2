#ifndef GENERICVERTEXBUFFER_H
#define GENERICVERTEXBUFFER_H

#include "GLState.h"
#include "VertexFormat.h"
#include "GpuBuffer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    template <class T>
    class GenericVertexBuffer : public GpuBuffer<T, GpuBufferPolicy::ArrayBuffer>
    {
    public:
        using Type = T;

        GenericVertexBuffer() = default;
        ~GenericVertexBuffer() = default;

        void setupVertexAttribPointer(uint offset) const;

        void create(size_t, T*, VertexFormat format = VNCT, DrawMode mode = STATIC, bool useInstancedAttrib = false);
        VertexFormat format() const;

        using GpuBuffer<T, GpuBufferPolicy::ArrayBuffer>::elementSize;
        using GpuBuffer<T, GpuBufferPolicy::ArrayBuffer>::id;

    protected:
        VertexFormat _format=VNC;
        bool _useInstancedAttrib = false;

        #include "MemoryLoggerOff.h"
        GenericVertexBuffer(const GenericVertexBuffer&) = delete;
        GenericVertexBuffer& operator=(const GenericVertexBuffer&) = delete;
        #include "MemoryLoggerOn.h"

        void bindVertexAttrib(uint, uint, uint) const;

    private:
        using GpuBuffer<T, GpuBufferPolicy::ArrayBuffer>::_elementSize;

    };

    using VertexBuffer = GenericVertexBuffer<float>;

    template <class T> VertexFormat GenericVertexBuffer<T>::format() const { return _format; }

    template <class T> void GenericVertexBuffer<T>::create(size_t size, T* data, VertexFormat format, DrawMode mode, bool useInstancedAttrib)
    {
        _useInstancedAttrib = useInstancedAttrib;
        _elementSize = vertexFormatSize(format);
        _format = format;
        GpuBuffer<T, GpuBufferPolicy::ArrayBuffer>::create(size, data, mode);
    }

    template <class T> void GenericVertexBuffer<T>::setupVertexAttribPointer(uint offset) const
    {
        openGL.bindVertexBuffer(id());
        switch (_format)
        {
            case VNCT: bindVertexAttrib(offset+3, 3, 8);
            case VNC:  bindVertexAttrib(offset+2, 2, 6);
            case VN:   bindVertexAttrib(offset+1, 3, 3);
            case V:    bindVertexAttrib(offset+0, 3, 0);
                break;

            case VC:
                bindVertexAttrib(offset+0, 3, 0);
                bindVertexAttrib(offset+1, 2, 3);
                break;

            case VEC1:
                bindVertexAttrib(offset+0, 1, 0);
            default: break;
        }
    }

    template <class T> void GenericVertexBuffer<T>::bindVertexAttrib(uint position, uint nbComponent, uint offset) const
    {
        glEnableVertexAttribArray(position);

        if(nbComponent)
        {
            static_assert(std::is_same<float, T>::value || std::is_same<int, T>::value, "Non supported template type for class GenericVertexBuffer");
            if(std::is_same<float, T>::value)
                glVertexAttribPointer(position, nbComponent, GL_FLOAT, 0, _elementSize*sizeof(float), BUFFER_OFFSET(offset*sizeof(float)));
            else if(std::is_same<int, T>::value)
                glVertexAttribIPointer(position, nbComponent, GL_INT, _elementSize*sizeof(int), BUFFER_OFFSET(offset*sizeof(int)));
        }

        if(_useInstancedAttrib)
            glVertexAttribDivisor(position, 1);
    }

}
}
#include "MemoryLoggerOff.h"

#endif // GENERICVERTEXBUFFER_H
