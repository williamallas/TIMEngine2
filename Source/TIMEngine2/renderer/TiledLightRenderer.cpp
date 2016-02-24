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

    _computeShader->bind();
    _nbLightUniformId = _computeShader->uniformLocation("nbLight");
}

TiledLightRenderer::~TiledLightRenderer()
{
    delete _computeShader;
}

void TiledLightRenderer::draw(const vector<Light>& lights)
{
    createLigthBuffer(lights);
    _lightBuffer.bind(0);

    _computeShader->bind();
    _computeShader->setUniform(static_cast<int>(lights.size()), _nbLightUniformId);

    openGL.bindImageTexture(_buffer->id(), 0, GL_WRITE_ONLY, Texture::toGLFormat(_buffer->format()));
    glDispatchCompute(_tileCount.x(),_tileCount.y(),1);

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void TiledLightRenderer::createLigthBuffer(const vector<Light>& lights)
{
    if(lights.empty())
        return;

    if(lights.size() > _lightBuffer.size())
        _lightBuffer.create(lights.size(), nullptr, DYNAMIC);


    vector<Std140LightData> data(lights.size());
    for(uint i=0 ; i<lights.size() ; ++i)
    {
        data[i].head = vec4(static_cast<float>(lights[i].type), lights[i].radius, lights[i].power, 0);
        data[i].position = vec4(lights[i].position);
        data[i].color = lights[i].color;
    }

     _lightBuffer.flush(&data[0], 0, lights.size());
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
