#ifndef MESHBUFFERS_H_INCLUDED
#define MESHBUFFERS_H_INCLUDED

#include "renderer.h"
#include "VAO.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    class MeshBuffers : boost::noncopyable
    {
    public:
        MeshBuffers(VBuffer* vb, IBuffer* ib) : _vb(vb), _ib(ib) {}

        ~MeshBuffers()
        {
            delete _vb;
            delete _ib;
        }

        void draw(size_t s, VertexMode primitive, size_t nbInstance, const VAO* vao = nullptr) const
        {
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

    private:
        VBuffer* _vb;
        IBuffer* _ib;
    };
}
}
#include "MemoryLoggerOff.h"

#endif // MESHBUFFERS_H_INCLUDED
