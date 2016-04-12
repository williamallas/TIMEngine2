#include "TerrainRenderer.h"
#include "interface/ResourceManager.h"

#include "Rand.h"

using namespace interface;

TerrainRenderer::TerrainRenderer(float pSize, float zScale, interface::SimpleScene& scene) : _patchSize(pSize), _scene(scene)
{
    resource::TextureLoader::ImageFormat imgFormat;
    ubyte* data = resource::textureLoader->loadImage("heightmap.png", imgFormat);
    vec4* fdata = new vec4[imgFormat.size.dot(imgFormat.size)];
    for(uint i=0 ; i<imgFormat.size.x() ; ++i)
        for(uint j=0 ; j<imgFormat.size.y() ; ++j)
            for(uint k=0 ; k<std::min(3u, imgFormat.nbComponent) ; ++k)
                fdata[i*imgFormat.size.y()+j][k] = float(data[(i*imgFormat.size.y()+j)*3+k]) / 255.f;
    delete[] data;

    renderer::Texture::GenTexParam param = Texture::genParam(false, true, false, false);
    param.format = renderer::Texture::Format::RGBA16F;
    param.nbLevels = 1;
    param.size = uivec3(imgFormat.size,0);
    renderer::Texture* tex = renderer::Texture::genTexture2D(param, reinterpret_cast<float*>(fdata), 4);
    tex->makeBindless();

    _patch = new Patch(_scene, 16, vec2(_patchSize, zScale), Texture(tex));
}

TerrainRenderer::Patch::Patch(interface::SimpleScene& scene, uint cellRes, vec2 size, const interface::Texture& hm)
    : _scene(scene), _resolution(cellRes), _sizeXY(size.x()), _sizeZ(size.y()), _heightMap(hm)
{
    _patch = new MeshInstance*[cellRes*cellRes];
    for(uint i=0 ; i<cellRes*cellRes ; ++i)
        _patch[i] = nullptr;

    Mesh::Element mesh;
    mesh.drawState().setShader(shaderManager.get("terrain"));
    mesh.setGeometry(geometryManager.load<false>("terrainCell_16.obj").value());

    Material mat;
    mat.header = 0;
    mat.textures[0] = _heightMap.handle();
    mat.textures[1] = textureManager.load<false>("grass_pure.png", Texture::genParam(true)).value().handle();

    float cellSize = _sizeXY / _resolution;
    for(uint x=0 ; x<cellRes ; ++x)
        for(uint y=0 ; y<cellRes ; ++y)
    {
        vec2 hpos = vec2(cellSize * x, cellSize * y);
        mat4 m = mat4::Scale(vec3::construct(cellSize*0.5));
        m.setTranslation(vec3(hpos,0));
        mat.offsetXY_size = vec3(0,0, _sizeXY);

        mesh.copyUserDefinedMaterial(mat);
        Mesh tmpMesh(mesh);
        tmpMesh.setInitialVolume(Sphere({0,0,10}, 20));
        _patch[x*_resolution+y] = &_scene.add<MeshInstance>(tmpMesh, m);
    }
}

TerrainRenderer::Patch::~Patch()
{
    delete[] _patch;
}

