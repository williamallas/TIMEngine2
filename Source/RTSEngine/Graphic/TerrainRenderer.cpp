#include "TerrainRenderer.h"
#include "interface/ResourceManager.h"
#include "ImageAlgorithm.h"
#include "resource/Image.h"

#include "Rand.h"

using namespace tim;
using namespace interface;

TerrainRenderer::TerrainRenderer(float pSize, float zScale, interface::SimpleScene& scene) : _scene(scene)
{
    _parameter.cellResolution = 4;
    _parameter.offset = vec3(-vec2::construct(pSize*0.5),0);
    _parameter.sharpness = 200;
    _parameter.size = pSize;
    _parameter.zscale = zScale;

    _terrainInfo.offset = _parameter.offset.to<2>();
    _terrainInfo.sharpness = _parameter.sharpness;
    _terrainInfo.vRes = _parameter.cellResolution * 64;
    _terrainInfo.XY_size = _parameter.size;
    _terrainInfo.zscale = _parameter.zscale;
    _uboTerrainInfo.create(1, &_terrainInfo);

    _patch = new Patch(_scene, _parameter, _uboTerrainInfo.id());

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

    _patch->setHeightData(f_img.blur<13>()/*.transformed(imat2::FLIP_Y())*/);
    _patch->generateHeightmap();
}

void TerrainRenderer::Patch::setHeightData(const ImageAlgorithm<vec3>& img)
{
    _gpuHeightData = img;
    uivec2 newSize(std::min(img.size().x(),256u), std::min(img.size().y(),256u));
    _heightData = _gpuHeightData.resized(newSize);
}

TerrainRenderer::Patch::Patch(interface::SimpleScene& scene, const Parameter& p, uint terrainUbo)
    : _scene(scene), _param(p)
{
    _patch = new MeshInstance*[_param.cellResolution*_param.cellResolution];
    for(uint i=0 ; i<_param.cellResolution*_param.cellResolution ; ++i)
        _patch[i] = nullptr;

    float cellSize = _param.size / _param.cellResolution;
    for(uint x=0 ; x<_param.cellResolution ; ++x)
        for(uint y=0 ; y<_param.cellResolution ; ++y)
    {
        vec2 hpos = vec2(cellSize * x + cellSize*0.5, cellSize * y + cellSize*0.5);
        mat4 m = mat4::Scale(vec3::construct(cellSize*0.5));
        m.setTranslation(vec3(hpos,0) + _param.offset);

        _patch[x*_param.cellResolution+y] = &_scene.add<MeshInstance>(Mesh(), m);
        _patch[x*_param.cellResolution+y]->attachUBO(terrainUbo, 0);
    }
}

void TerrainRenderer::Patch::generateHeightmap()
{
    renderer::Texture::GenTexParam param = Texture::genParam(false, true, false, false);
    param.format = renderer::Texture::Format::RGBA16F;
    param.nbLevels = 1;
    param.size = uivec3(_gpuHeightData.size(),0);

    renderer::Texture* tex = renderer::Texture::genTexture2D(param, reinterpret_cast<float*>(_gpuHeightData.data()), 3);
    tex->makeBindless();
    _heightMap = Texture(tex);

    Mesh::Element elem;
    elem.drawState().setShader(shaderManager.get("terrain"));
    elem.setGeometry(geometryManager.load<false>("terrainCell_64.obj").value());

    Material mat;
    mat.textures[0] = _heightMap.handle();
    mat.textures[1] = textureManager.load<false>(vector<std::string>({"ground.png", "grass1.png","grass2.png"}),
                                                 Texture::genParam(true,true,true,8)).value().handle();
    mat.textures[2] = textureManager.load<false>("mask.png", Texture::genParam()).value().handle();

    mat.scales[0] = 30;
    mat.scales[1] = 20;
    mat.scales[2] = 20;

    elem.copyUserDefinedMaterial(mat);

    Mesh mesh(elem);
    //mesh.setInitialVolume(Sphere({0,0,10}, 20));

    for(uint x=0 ; x<_param.cellResolution ; ++x)
        for(uint y=0 ; y<_param.cellResolution ; ++y)
    {
        _patch[x*_param.cellResolution+y]->setMesh(mesh);
    }
}

TerrainRenderer::Patch::~Patch()
{
    delete[] _patch;
}

float TerrainRenderer::Patch::height(vec2 v) const
{
    v -= _param.offset.to<2>();
    v /= _param.size;

    std::swap(v.x(), v.y());

    v *= vec2(_gpuHeightData.size().x()-1, _gpuHeightData.size().y()-1);
    return _gpuHeightData.getLinear(v).z() * _param.zscale;
}

vec3 TerrainRenderer::Patch::normal(vec2 v) const
{
    vec3 a = vec3(v+vec2(-0.25,0), height(v+vec2(-0.25,0)));
    vec3 b = vec3(v+vec2(0.25,0.25), height(v+vec2(0.25,0.25)));
    vec3 c = vec3(v+vec2(0.25,-0.25), height(v+vec2(0.25,-0.25)));

    vec3 n = Plan(a,b,c).plan().to<3>().normalized();
    std::swap(n.x(), n.y());
    n.x() *= -1;
    return n;
}

