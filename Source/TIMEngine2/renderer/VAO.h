#ifndef VAO_H_INCLUDED
#define VAO_H_INCLUDED

#include "GLState.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    class VAO : public boost::noncopyable
    {
    public:

        template<class ...Args>
        VAO(const Args& ...args)
        {
            glGenVertexArrays(1, &_id);
            openGL.bindVao(_id);

            inner_create(0, args...);

            openGL.bindVao(0);
        }

        ~VAO()
        {
            glDeleteVertexArrays(1, &_id);
        }

        void bind() const { openGL.bindVao(_id); }

        uint id() const { return _id; }

    private:
        uint _id=0;

        template<class BufferType, class ...Args>
        void inner_create(int counter, const BufferType& arg, const Args& ...args)
        {
            arg.setupVertexAttribPointer(counter);
            inner_create(counter+vertexFormatOffset(arg.format()), args...);
        }

        void inner_create(int) {}
    };
}
}
#include "MemoryLoggerOff.h"

#endif // VAO_H_INCLUDED
