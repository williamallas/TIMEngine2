#include "IndexBuffer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

const uint IndexBuffer::GLPrimitive[6]={GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_LINES, GL_LINE_STRIP, GL_POINTS, GL_QUADS}; // see VertexMode

void IndexBuffer::draw(size_t s, VertexMode primitive, size_t nbInstance=0) const
{
    bind();
    s = std::min(s, size());

    if(nbInstance>0)
        glDrawElementsInstanced(IndexBuffer::GLPrimitive[primitive], s, GL_UNSIGNED_INT, 0, nbInstance);
    else
        glDrawElements(IndexBuffer::GLPrimitive[primitive], s, GL_UNSIGNED_INT, 0);
}

}
}
