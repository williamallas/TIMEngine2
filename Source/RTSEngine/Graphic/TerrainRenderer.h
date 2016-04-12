#ifndef TERRAINRENDERER_H
#define TERRAINRENDERER_H

#include "interface/SimpleScene.h"
#include "interface/MeshInstance.h"

using namespace tim;

class TerrainRenderer
{
public:
    class Patch
    {
    public:
        Patch(interface::SimpleScene&, uint, vec2, const interface::Texture&);
        ~Patch();

    private:
        interface::SimpleScene& _scene;
        uint _resolution;
        float _sizeXY, _sizeZ;
        interface::Texture _heightMap;
        interface::MeshInstance** _patch;
    };

    TerrainRenderer(float, float, interface::SimpleScene&);

private:
    struct Material
    {
        float header; vec3 offsetXY_size;
        uint64_t textures[4];

        vec4 other;
    };
    static_assert(sizeof(Material) == sizeof(renderer::DummyMaterial), "internal terrain Material is not well sized.");

    float _patchSize;
    interface::SimpleScene& _scene;

    Patch* _patch; // actually only a single patch
};

#endif // TERRAINRENDERER_H
