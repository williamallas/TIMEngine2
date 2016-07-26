#ifndef pipelineDEFERREDRENDERER_NODE_H
#define pipelineDEFERREDRENDERER_NODE_H

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
    class DeferredRendererNode : public Pipeline::ObjectRendererNode
    {
    public:
        DeferredRendererNode(renderer::MeshRenderer& meshDrawer)
            : Pipeline::ObjectRendererNode(), _meshDrawer(meshDrawer) {}

        renderer::Texture* buffer(uint) const override;
        void prepare() override;
        void render() override;

        void setRendererEntity(Pipeline::DeferredRendererEntity&);
        void setScissorTest(bool b, vec2 coord = {0,0}, vec2 size = {1,1});

        bool isAuxiliar() const { return _isAux; }
        void setAuxiliar(bool b) { _isAux = b; }

    private:
        renderer::MeshRenderer& _meshDrawer;

        vector<std::reference_wrapper<LightInstance>> _culledLight;

        bool _useScissor = false;
        vec2 _coordScissor = {0,0};
        vec2 _sizeScissor = {1,1};
        bool _isAux = false;

        struct EInst
        {
            const Mesh::Element* elem;
            const mat4* matrix;
            const vector<uint>* extraUbo;
        };

        using ElementInstance = EInst;//std::pair<const Mesh::Element*, const mat4*>;
        vector<ElementInstance> _toDraw;

        Pipeline::DeferredRendererEntity* _rendererEntity = nullptr;
        renderer::FrameBuffer* _copyToFBO = nullptr;
        renderer::Texture* _copyBuffer = nullptr;
    };
}

}
}
#include "MemoryLoggerOff.h"

#endif // pipelineDEFERREDRENDERER_NODE_H
