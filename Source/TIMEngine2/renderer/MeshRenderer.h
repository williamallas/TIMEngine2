#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include "GLState.h"
#include "DrawState.h"
#include "GpuBuffer.h"
#include "core/Camera.h"
#include "MeshBuffers.h"
#include "FrameParameter.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    class MeshRenderer
    {
    public:
        MeshRenderer(const FrameParameter&);
        virtual ~MeshRenderer();

        void bind() const;
        int draw(const vector<MeshBuffers*>&, const vector<mat4>&, const vector<Material>& mat = {});

        void setDrawState(const DrawState&);

    private:
        const uint _maxUboMa4;
        DrawState _states;
        const FrameParameter& _parameter;

        GenericVertexBuffer<int> _drawIdBuffer;
        VAO* _vao = nullptr;

#ifdef USE_SSBO_MODELS
        ShaderStorageBuffer<mat4> _modelBuffer;
#else
        UniformBuffer<mat4> _modelBuffer;
        UniformBuffer<Material> _materialBuffer;
#endif
    };



}
}
#include "MemoryLoggerOff.h"

#endif // FRAMEBUFFER_H
