#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include "GLState.h"
#include "BufferPool.h"
#include "IndexBuffer.h"
#include "GenericVertexBuffer.h"
#include "Shader.h"

namespace tim
{
    using namespace core;
namespace renderer
{
    bool init();
    bool close();

    using VertexBufferPoolType = BufferPool<VertexBuffer, 64>;
    using IndexBufferPoolType = BufferPool<IndexBuffer, 256>;

    extern VertexBufferPoolType* vertexBufferPool;
    extern IndexBufferPoolType* indexBufferPool;

    using VBuffer = VertexBufferPoolType::Instance;
    using IBuffer = IndexBufferPoolType::Instance;

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

    struct Material
    {
        uint64_t id;
        uint64_t texures[3];
        vec4 parameter;
        vec4 color;
    };
    static_assert(sizeof(Material) <= sizeof(mat4), "Material struct is too big.");

    extern const char* drawQuad_vertex;
    extern const char* drawQuad_pixel;
    extern Shader* drawQuadShader;

    class MeshBuffers;
    extern MeshBuffers* quadMeshBuffers;
}
}

#endif // RENDERER_H_INCLUDED
