#include "TerrainRenderer.h"
#include "interface/ResourceManager.h"
#include "ImageAlgorithm.h"

#include "Rand.h"

using namespace interface;

TerrainRenderer::TerrainRenderer(float pSize, float zScale, interface::SimpleScene& scene) : _patchSize(pSize), _scene(scene)
{
    _terrainInfo.offset = {0,0};
    _terrainInfo.sharpness = 200;
    _terrainInfo.vRes = 64*8;
    _terrainInfo.XY_size = pSize;
    _terrainInfo.zscale = zScale;
    _uboTerrainInfo.create(1, &_terrainInfo);

    _patch = new Patch(_scene, 8, vec2(_patchSize, zScale), _uboTerrainInfo.id());

    resource::TextureLoader::ImageFormat imgFormat;
    ubyte* data = resource::textureLoader->loadImage("heightmap.png", imgFormat);

    ImageAlgorithm<vec3> f_img(imgFormat.size);
    for(uint i=0 ; i<imgFormat.size.x() ; ++i)
        for(uint j=0 ; j<imgFormat.size.y() ; ++j)
            for(uint k=0 ; k<std::min(3u, imgFormat.nbComponent) ; ++k)
            {
                f_img.get(i,j)[k] = float(data[(i*imgFormat.size.y()+j)*imgFormat.nbComponent+k]) / 255.f;
            }

    delete[] data;

    _patch->heightData() = f_img.blur<7>();
    _patch->generateHeightmap();
}

TerrainRenderer::Patch::Patch(interface::SimpleScene& scene, uint cellRes, vec2 size, uint terrainUbo)
    : _scene(scene), _resolution(cellRes), _sizeXY(size.x()), _sizeZ(size.y())
{
    _patch = new MeshInstance*[cellRes*cellRes];
    for(uint i=0 ; i<cellRes*cellRes ; ++i)
        _patch[i] = nullptr;

    float cellSize = _sizeXY / _resolution;
    for(uint x=0 ; x<cellRes ; ++x)
        for(uint y=0 ; y<cellRes ; ++y)
    {
        vec2 hpos = vec2(cellSize * x, cellSize * y);
        mat4 m = mat4::Scale(vec3::construct(cellSize*0.5));
        m.setTranslation(vec3(hpos,0));

        _patch[x*_resolution+y] = &_scene.add<MeshInstance>(Mesh(), m);
        _patch[x*_resolution+y]->attachUBO(terrainUbo, 0);
    }
}

void TerrainRenderer::Patch::generateHeightmap()
{
    renderer::Texture::GenTexParam param = Texture::genParam(false, true, false, false);
    param.format = renderer::Texture::Format::RGBA16F;
    param.nbLevels = 1;
    param.size = uivec3(_heightData.size(),0);

    renderer::Texture* tex = renderer::Texture::genTexture2D(param, reinterpret_cast<float*>(_heightData.data()), 3);
    tex->makeBindless();
    _heightMap = Texture(tex);

    Mesh::Element elem;
    elem.drawState().setShader(shaderManager.get("terrain"));
    elem.setGeometry(geometryManager.load<false>("terrainCell_64.obj").value());

    Material mat;
    mat.textures[0] = _heightMap.handle();
    mat.textures[1] = textureManager.load<false>(vector<std::string>({"grass1.png","grass2.png","ground.png"}),
                                                 Texture::genParam()).value().handle();
    mat.scales[0] = 30;
    mat.scales[1] = 30;
    mat.scales[2] = 60;

    elem.copyUserDefinedMaterial(mat);

    Mesh mesh(elem);
    mesh.setInitialVolume(Sphere({0,0,10}, 20));

    for(uint x=0 ; x<_resolution ; ++x)
        for(uint y=0 ; y<_resolution ; ++y)
    {
        _patch[x*_resolution+y]->setMesh(mesh);
    }
}

TerrainRenderer::Patch::~Patch()
{
    delete[] _patch;
}

