#ifndef ONSCREENRENDERER_NODE_H
#define ONSCREENRENDERER_NODE_H

#include "interface/Pipeline.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
namespace pipeline
{
    class OnScreenRenderer : public Pipeline::TerminalNode
    {
    public:
        OnScreenRenderer();
        ~OnScreenRenderer() = default;

        void prepare() override;
        void render() override;

    private:
        renderer::DrawState _stateDrawQuad;
    };
}
}
}
#include "MemoryLoggerOff.h"

#endif // ONSCREENRENDERER_NODE_H
