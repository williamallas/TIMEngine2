#ifndef TERRAINRENDERER_H
#define TERRAINRENDERER_H

#include "interface/SimpleScene.h"
#include "interface/MeshInstance.h"
#include "ImageAlgorithm.h"

class TerrainRenderer
{
public:
    class Patch
    {
    public:
        Patch(interface::SimpleScene&, uint, vec2, uint);
        ~Patch();

        ImageAlgorithm<vec3>& heightData() { return _heightData; }
        void generateHeightmap();

    private:
        interface::SimpleScene& _scene;
        uint _resolution;
        uint _uboId;
        float _sizeXY, _sizeZ;

        interface::Texture _heightMap;
        ImageAlgorithm<vec3> _heightData;
        interface::MeshInstance** _patch;
    };

    TerrainRenderer(float, float, interface::SimpleScene&);

private:
    struct Material
    {
        uint64_t textures[4];
        float scales[8] = {10};
    };
    static_assert(sizeof(Material) == sizeof(renderer::DummyMaterial), "internal terrain Material is not well sized.");

    float _patchSize;
    interface::SimpleScene& _scene;

    struct TerrainInfo
    {
        vec2 offset;
        float zscale;
        float sharpness;
        float XY_size;
        float vRes;
    };
    TerrainInfo _terrainInfo;
    renderer::UniformBuffer<TerrainInfo> _uboTerrainInfo;

    Patch* _patch; // actually only a single patch
};

#endif // TERRAINRENDERER_H
