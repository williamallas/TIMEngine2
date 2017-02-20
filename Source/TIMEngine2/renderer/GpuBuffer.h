#ifndef GPUBUFFER_H
#define GPUBUFFER_H

#include "GLState.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    enum DrawMode { STATIC, STREAM, DYNAMIC };

    template
    <
        typename T,
        class BufferPolicy
    >
    class GpuBuffer : boost::noncopyable
    {
    public:
        using Type = T;

        GpuBuffer() {}

        virtual ~GpuBuffer()
        {
            freeBuffer();
        }

        void create(size_t size, const T* data, DrawMode mode = STATIC)
        {
            _size = size;
            if(_bufferId == 0)
                glGenBuffers(1, &_bufferId);

            BufferPolicy::bind(_bufferId);

            GLenum m = GL_STATIC_DRAW;
            switch(mode)
            {
                case STATIC : m = GL_STATIC_DRAW;  break;
                case STREAM : m = GL_STREAM_DRAW;  break;
                case DYNAMIC: m = GL_DYNAMIC_DRAW; break;
            }

            glBufferData(BufferPolicy::BUFFER_TYPE,
                        _elementSize*_size*sizeof(T),
                        data, m);
        }

        void flush(const T* data, size_t begin, size_t size) const
        {
            BufferPolicy::bind(_bufferId);

            glBufferSubData(BufferPolicy::BUFFER_TYPE,
                            begin*_elementSize*sizeof(T),
                            size*_elementSize*sizeof(T),
                            data);
        }

        uint id() const { return _bufferId; }
        size_t size() const { return _size; }
        size_t elementSize() const { return _elementSize; }

        void freeBuffer()
        {
            uint id = _bufferId;

            openGL.pushGLTask([=]()
            {
                BufferPolicy::unbind(id);
                glDeleteBuffers(1, &id);
            });

            _bufferId = 0;
            _size = 0;
            _elementSize = 0;
        }

    private:
        uint _bufferId=0;
        size_t _size=0;

    protected:
        size_t _elementSize=1;
    };

    namespace GpuBufferPolicy
    {
        struct ArrayBuffer
        {
            static void bind(uint id) { openGL.bindVertexBuffer(id); }
            static void unbind(uint id) { openGL.unbindVertexBuffer(id); }
            static const GLenum BUFFER_TYPE = GL_ARRAY_BUFFER;
        };

        struct ElementArrayBuffer
        {
            static void bind(uint id) { openGL.bindElementArrayBuffer(id); }
            static void unbind(uint id) { openGL.unbindElementArrayBuffer(id); }
            static const GLenum BUFFER_TYPE = GL_ELEMENT_ARRAY_BUFFER;
        };

        struct ShaderStorageBuffer
        {
            static void bind(uint id) { openGL.bindShaderStorageBuffer(id); }
            static void unbind(uint id) { openGL.unbindShaderStorageBuffer(id); }
            static const GLenum BUFFER_TYPE = GL_SHADER_STORAGE_BUFFER;
        };

        struct UniformBuffer
        {
            static void bind(uint id) { openGL.bindUniformBuffer(id); }
            static void unbind(uint id) { openGL.unbindUniformBuffer(id); }
            static const GLenum BUFFER_TYPE = GL_UNIFORM_BUFFER;
        };

        struct MultiDrawBuffer
        {
            static void bind(uint id) { openGL.bindDrawIndirectBuffer(id); }
            static void unbind(uint id) { openGL.unbindDrawIndirectBuffer(id); }
            static const GLenum BUFFER_TYPE = GL_DRAW_INDIRECT_BUFFER;
        };
    }

    template <class T, class Policy>
    class BindableGpuBuffer : public GpuBuffer<T, Policy>
    {
    public:
        BindableGpuBuffer() = default;
        ~BindableGpuBuffer() = default;

        using GpuBuffer<T, Policy>::id;
        const BindableGpuBuffer& bind(uint index) const { openGL.bindShaderStorageBuffer(id(), index); return *this; }
    };

    template <class T>
    using ShaderStorageBuffer = BindableGpuBuffer<T, GpuBufferPolicy::ShaderStorageBuffer >;

    template <class T>
    using UniformBuffer = GpuBuffer<T, GpuBufferPolicy::UniformBuffer >;

}
}
#include "MemoryLoggerOff.h"

#endif // GPUBUFFER_H
