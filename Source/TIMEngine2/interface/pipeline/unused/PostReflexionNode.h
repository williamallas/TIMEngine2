#ifndef POSTREFLEXION_NODE_H
#define POSTREFLEXION_NODE_H

#include "interface/Pipeline.h"
#include "renderer/PostReflexionRenderer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
namespace pipeline
{
    class PostReflexionNode : public Pipeline::InOutBufferNode
    {
    public:
        PostReflexionNode(const renderer::FrameParameter&);
        ~PostReflexionNode() {}

        renderer::Texture* buffer() const override { return _reflexionRenderer.buffer(); }

        void prepare() override;
        void render() override;

    private:
        renderer::PostReflexionRenderer _reflexionRenderer;
    };
}
}
}
#include "MemoryLoggerOff.h"

#endif // POSTREFLEXION_NODE_H
