#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include "GLState.h"
#include "DrawState.h"
#include "GpuBuffer.h"
#include "core/Camera.h"
#include "MeshBuffers.h"

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
        virtual ~MeshRenderer();

        void bind() const;
        int draw(const vector<MeshBuffers*>&, const vector<mat4>&, const vector<Material>& mat = {});

        void setCamera(const Camera&);
        void setViewMatrix(const mat4&, const vec3&);
        void setWordlOrigin(const vec3&);
        void setProjectionMatrix(const mat4&);
        void setTime(float, float);

        void setDrawState(const DrawState&);

    private:
        const uint _maxUboMa4;
        DrawState _states;

        GenericVertexBuffer<int> _drawIdBuffer;
        VAO* _vao = nullptr;

        struct Parameter // std140
        {
            mat4 view, proj;
            mat4 projView, invView, invProj, invViewProj, worldOriginInvViewProj;
            vec4 cameraPos, cameraUp, cameraDir, worldOrigin;
            vec4 time;
        };

        Parameter _parameter;
        bool _hasChanged;

        UniformBuffer<Parameter> _uboParameter;

#ifdef USE_SSBO_MODELS
        ShaderStorageBuffer<mat4> _modelBuffer;
#else
        UniformBuffer<mat4> _modelBuffer;
        UniformBuffer<Material> _materialBuffer;
#endif

        void updateParameter();
    };



}
}
#include "MemoryLoggerOff.h"

#endif // FRAMEBUFFER_H
