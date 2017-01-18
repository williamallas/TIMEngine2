#ifndef pipelineDIRLIGHTSHADOWRENDERER_NODE_H
#define pipelineDIRLIGHTSHADOWRENDERER_NODE_H

#include "interface/Pipeline.h"
#include "renderer/GLState.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
namespace pipeline
{
    class DirLightShadowNode : public Pipeline::DepthMapRendererNode
    {
    public:
        DirLightShadowNode(renderer::MeshRenderer&);
        ~DirLightShadowNode();

        renderer::Texture* buffer(uint) const override;
        void prepare() override;
        void render() override;

        void acquire(int) override;
        void release(int) override;

        void setShadowLightRange(const vector<float>&);
        void setDepthMapResolution(uint);

    private:
        renderer::MeshRenderer& _meshDrawer;

        using ElementInstance = std::pair<const Mesh::Element*, const mat4*>;

        vec3 _sizeOrtho[renderer::MAX_SHADOW_MAP_LVL];
        mat4 _orthoMatrix[renderer::MAX_SHADOW_MAP_LVL];
        vector<ElementInstance> _toDraw[renderer::MAX_SHADOW_MAP_LVL];

        bool _needUpdate = true;
        int _counter = 0;
        uivec3 _resolution = {1024,1024,3};
        renderer::DrawState _defaultDrawState;

        renderer::PooledBuffer _buffer;
    };
}

}
}
#include "MemoryLoggerOff.h"

#endif // pipelineDIRLIGHTSHADOWRENDERER_NODE_H
