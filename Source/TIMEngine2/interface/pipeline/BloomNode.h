#ifndef BLOOMNODE_H
#define BLOOMNODE_H

#include "interface/Pipeline.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
namespace pipeline
{
    class BloomNode : public Pipeline::InOutBufferNode
    {
    public:
        BloomNode();
        ~BloomNode() {}

        renderer::Texture* buffer() const override { return nullptr; }

        void prepare() override;
        void render() override;

    private:

    };
}
}
}
#include "MemoryLoggerOff.h"

#endif // POSTREFLEXION_NODE_H
