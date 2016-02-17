#include "TiledLightRenderer.h"
#include "ShaderCompiler.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

TiledLightRenderer::TiledLightRenderer(DeferredRenderer& deferred, bool hdr) : LightContextRenderer(deferred)
{
    _tileCount = deferred.resolution() / TILE_SIZE;
    if(deferred.resolution().x() % TILE_SIZE.x() != 0)
        _tileCount.x()++;
    if(deferred.resolution().y() % TILE_SIZE.y() != 0)
        _tileCount.y()++;

    ShaderCompiler shader(ShaderCompiler::COMPUTE_SHADER);
    shader.setSource(StringUtils::readFile("shader/tiledLightShader.cs"));

    auto optShader = Shader::linkComputeShader(shader.compile({}));

    if(!optShader.hasValue())
    {
        LOG("shader/tiledLightShader.cs error:\n", shader.error());
        LOG("Link erorr:", Shader::lastLinkError());
    }
    else
        _computeShader = optShader.value();

//    _shader = shader;
//    _shader->bind();
//    _nbLightUniformId = _shader->uniformLocation("nbLight");
}

TiledLightRenderer::~TiledLightRenderer()
{
    delete _computeShader;
}

//void TiledLightRenderer::draw(const vector<LightInstance>& lights)
//{

//}

//void TiledLightRenderer::buildLightBuffer(const vector<LightInstance>& lights)
//{
//    if(lights.empty())
//        return;

//    bool newBuffer = false;

//    if(lights.size() > _lightBuffer.size())
//    {
//        newBuffer = true;
//        _lightBuffer.createBuffer(lights.size());
//    }

//    for(uint i=0 ; i<lights.size() ; ++i)
//    {
//        Light* l = lights[i].light();
//        Std140LightData tmp;
//        tmp.attenuation = l->lightData().attenuation;
//        tmp.diffuse = l->lightData().diffuse;
//        tmp.specular = l->lightData().specular;
//        tmp.spotData = l->lightData().spotData;
//        tmp.position = vec4(lights[i].matrix()->translation());
//        tmp.head = {(float)l->lightData().type, l->radius(),0,0};
//        _lightBuffer.data()[i] = tmp;
//    }

//    if(newBuffer)
//        _lightBuffer.uploadOnGpu(true, false);
//    else
//        _lightBuffer.flushOnGpu(0, lights.size());
//}

}
}
