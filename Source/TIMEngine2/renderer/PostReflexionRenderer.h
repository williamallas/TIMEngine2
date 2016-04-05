#ifndef POSRREFLEXIONRENDERER_H
#define POSRREFLEXIONRENDERER_H

#include "LightContextRenderer.h"
#include "DrawState.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

    class PostReflexionRenderer : boost::noncopyable
    {
    public:
        PostReflexionRenderer(LightContextRenderer&);
        ~PostReflexionRenderer();

        void draw();

    private:
        LightContextRenderer& _context;
        Shader* _reflexionShader;
        DrawState _state;
    };
}
}
#include "MemoryLoggerOff.h"

#endif // DEFERREDRENDERER_H
