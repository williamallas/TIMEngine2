#ifndef MESHBUFFERS_H_INCLUDED
#define MESHBUFFERS_H_INCLUDED

#include "renderer.h"
#include "VAO.h"
#include "Sphere.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    class MeshBuffers : boost::noncopyable
    {
    public:
        MeshBuffers(VBuffer* vb, IBuffer* ib, const Sphere& s = Sphere()) : _vb(vb), _ib(ib), _volume(s) {}

        ~MeshBuffers()
        {
            delete _vb;
            delete _ib;
        }

        void draw(size_t s, VertexMode primitive, size_t nbInstance, const VAO* vao = nullptr) const
        {
            if(!_vb || !_ib) return;

            if(vao)
                vao->bind();

            _ib->bind(); // bind Element
            s = std::min(s, _ib->size());

            if(nbInstance>0)
                glDrawElementsInstancedBaseVertex(IndexBuffer::GLPrimitive[primitive], s, GL_UNSIGNED_INT, BUFFER_OFFSET(_ib->offset()*sizeof(IBuffer::Type)), nbInstance, _vb->offset());
            else
                glDrawElementsBaseVertex(IndexBuffer::GLPrimitive[primitive], s, GL_UNSIGNED_INT, BUFFER_OFFSET(_ib->offset()*sizeof(IBuffer::Type)), _vb->offset());
        }

        VBuffer* vb() const { return _vb; }
        IBuffer* ib() const { return _ib; }

        bool isNull() const { return (!_vb || !_ib); }

        const Sphere& volume() const { return _volume; }
        void setVolume(const Sphere& s) { _volume = s; }

        void swap(MeshBuffers& buf)
        {
            std::swap(_vb, buf._vb);
            std::swap(_ib, buf._ib);
            std::swap(_volume, buf._volume);
        }

    private:
        VBuffer* _vb;
        IBuffer* _ib;

        Sphere _volume;
    };
}
}
#include "MemoryLoggerOff.h"

#endif // MESHBUFFERS_H_INCLUDED
