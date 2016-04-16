#ifndef TERRAINRENDERER_H
#define TERRAINRENDERER_H

#include "interface/SimpleScene.h"
#include "interface/MeshInstance.h"
#include "ImageAlgorithm.h"

class TerrainRenderer
{
public:
    struct Parameter
    {
        vec3 offset;
        float size;
        float zscale;
        float sharpness;
        uint cellResolution;
    };

    class Patch
    {
    public:
        Patch(tim::interface::SimpleScene&, const Parameter&, uint);
        ~Patch();

        ImageAlgorithm<vec3>& heightData() { return _heightData; }
        const ImageAlgorithm<vec3>& heightData() const { return _heightData; }
        void generateHeightmap();

    private:
        tim::interface::SimpleScene& _scene;
        const Parameter& _param;
        uint _uboId;

        tim::interface::Texture _heightMap;
        ImageAlgorithm<vec3> _heightData;
        tim::interface::MeshInstance** _patch;
    };

    TerrainRenderer(float, float, tim::interface::SimpleScene&);

    const Patch* patch() const { return _patch; }

private:
    struct Material
    {
        uint64_t textures[4];
        float scales[8] = {10};
    };
    static_assert(sizeof(Material) == sizeof(tim::renderer::DummyMaterial), "internal terrain Material is not well sized.");

    tim::interface::SimpleScene& _scene;

    struct UBOTerrainInfo
    {
        vec2 offset;
        float zscale;
        float sharpness;
        float XY_size;
        float vRes;
    };
    UBOTerrainInfo _terrainInfo;
    tim::renderer::UniformBuffer<UBOTerrainInfo> _uboTerrainInfo;

    Patch* _patch; // actually only a single patch

    Parameter _parameter;
};

#endif // TERRAINRENDERER_H
