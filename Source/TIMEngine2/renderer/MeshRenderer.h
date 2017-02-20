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
        MeshRenderer();
        ~MeshRenderer();

        void bind() const;
        int draw(const vector<MeshBuffers*>&, const vector<mat4>&, const vector<DummyMaterial>& mat = {},
                 const vector<vector<uint>>& extraUbo={}, bool useCameraUbo = true);

        void setDrawState(const DrawState&);

        FrameParameter& frameState();
        const FrameParameter& frameState() const;

    private:

#ifdef USE_VCPP
        static const uint _maxUboMat4 = 1024;
#else
        const uint _maxUboMat4;
#endif

        DrawState _states;
        FrameParameter _parameter;

        GenericVertexBuffer<int> _drawIdBuffer;
        VAO* _vao = nullptr;

#ifdef USE_SSBO_MODELS
        ShaderStorageBuffer<mat4> _modelBuffer;
#else
        UniformBuffer<mat4> _modelBuffer;

        UniformBuffer<DummyMaterial> _materialBuffer;
        GpuBuffer<IndirectDrawParmeter, GpuBufferPolicy::MultiDrawBuffer> _drawIndirectBuffer;
#endif
    };

    inline FrameParameter& MeshRenderer::frameState() { return _parameter; }
    inline const FrameParameter& MeshRenderer::frameState() const  { return _parameter; }

}
}
#include "MemoryLoggerOff.h"

#endif // FRAMEBUFFER_H
