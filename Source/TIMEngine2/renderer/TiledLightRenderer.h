#ifndef TILEDLIGHTRENDERER_H
#define TILEDLIGHTRENDERER_H

#include "LightContextRenderer.h"
#include "Shader.h"
#include "GpuBuffer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

    class TiledLightRenderer : public LightContextRenderer
    {
    public:
        TiledLightRenderer(DeferredRenderer&, bool hdr=false);
        ~TiledLightRenderer();

        void draw(const vector<Light>&);

    private:
        const uivec2 TILE_SIZE = {32,30};
        uivec2 _tileCount;

        Shader* _computeShader = nullptr;

        struct Std140LightData
        {
            vec4 head; // (type, radius, power, x)
            vec4 position;
            vec4 color;
        };
        renderer::ShaderStorageBuffer<Std140LightData> _lightBuffer;
        int _nbLightUniformId = -1;

        void createLigthBuffer(const vector<Light>&);
    };

}
}
#include "MemoryLoggerOff.h"

#endif // TILEDLIGHTRENDERER_H
