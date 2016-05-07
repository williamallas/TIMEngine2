#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include "core/ThreadPool.h"
#include "GLState.h"
#include "BufferPool.h"
#include "IndexBuffer.h"
#include "GenericVertexBuffer.h"
#include "Shader.h"
#include "Camera.h"

namespace tim
{
    using namespace core;
namespace renderer
{
    bool init();
    bool close();

    extern ThreadPool globalThreadPool;

    using VertexBufferPoolType = BufferPool<VertexBuffer, 64>;
    using IndexBufferPoolType = BufferPool<IndexBuffer, 256>;

    extern VertexBufferPoolType* vertexBufferPool;
    extern IndexBufferPoolType* indexBufferPool;

    using VBuffer = VertexBufferPoolType::Instance;
    using IBuffer = IndexBufferPoolType::Instance;

    using VertexType = VNCT_Vertex;

    struct TextureMode { enum : int { NoFilter=0, Filtered, FilteredNoRepeat, DepthMap, Last }; };
    extern uint textureSampler[TextureMode::Last];

    struct IndirectDrawParmeter
    {
        uint  count;
        uint  instanceCount;
        uint  firstIndex;
        uint  baseVertex;
        uint  baseInstance;
    };

    struct DummyMaterial
    {
        uint data[16];
    };

    struct Material
    {
        uivec2 header;
        uint64_t texures[3];
        vec4 parameter;
        vec4 color;
    };
    static_assert(sizeof(DummyMaterial) == sizeof(mat4), "DummyMaterial struct is too big or to small.");
    static_assert(sizeof(Material) == sizeof(DummyMaterial), "Material struct is too big or to small.");

    static const uint MAX_SHADOW_MAP_LVL = DirLightView::MAX_LVL_DIRLIGHT;

    extern const char* drawQuad_vertex;
    extern const char* drawQuad_pixel;
    extern Shader* drawQuadShader;

    extern const char* depthPass_vertex;
    extern Shader* depthPassShader;

    class MeshBuffers;
    extern MeshBuffers* quadMeshBuffers;
}
}

#endif // RENDERER_H_INCLUDED
