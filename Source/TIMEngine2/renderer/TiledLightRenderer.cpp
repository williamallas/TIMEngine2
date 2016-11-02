#include "TiledLightRenderer.h"
#include "ShaderCompiler.h"
#include "renderer.h"

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

    Texture::GenTexParam param;
    param.format = Texture::RGB16;
    param.nbLevels = 1;
    param.size = uivec3(256,256,1);
    float* dat = new float[256*256*3];
    std::ifstream inDat("shader/brdf_256.dat", std::ios_base::binary);
    inDat.read((char*)dat, sizeof(float)*256*256*3);
    _processedBrdf = Texture::genTexture2D(param, dat, 3);
    delete[] dat;
}

TiledLightRenderer::~TiledLightRenderer()
{
    delete _processedBrdf;
    delete _computeShader;
}

void TiledLightRenderer::draw(const vector<Light>& lights, Texture* processedSkybox)
{
    createLigthBuffer(lights);
    _lightBuffer.bind(0);

    _computeShader->bind();
    _computeShader->setUniform(static_cast<int>(lights.size()), _nbLightUniformId);

    openGL.bindImageTexture(_buffer.buffer(0)->id(), 0, GL_WRITE_ONLY, Texture::toGLFormat(_buffer.buffer(0)->format()));

    for(int i=0 ; i<4 ; ++i)
    {
        openGL.bindTextureSampler(textureSampler[TextureMode::NoFilter], i);
        _deferred.buffer(i)->bind(i);
    }

    if(_processedBrdf)
    {
        _processedBrdf->bind(4);
        openGL.bindTextureSampler(textureSampler[TextureMode::FilteredNoRepeat], 4);
    } else openGL.bindTexture(0, GL_TEXTURE_2D, 4);

    if(processedSkybox)
    {
        processedSkybox->bind(5);
        openGL.bindTextureSampler(textureSampler[TextureMode::FilteredNoRepeat], 5);
    } else openGL.bindTexture(0, GL_TEXTURE_CUBE_MAP, 5);

    uint texIndex=6;
    for(const Light& l : lights)
    {
        if(l.type == Light::SPECULAR_PROB && l.tex != nullptr)
        {
            openGL.bindTextureSampler(textureSampler[TextureMode::FilteredNoRepeat], texIndex);
            l.tex->bind(texIndex++);
        }
    }

    _frameState.bind(0);
    glDispatchCompute(_tileCount.x(),_tileCount.y(),1);

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void TiledLightRenderer::createLigthBuffer(const vector<Light>& lights)
{
    if(lights.empty())
        return;

    if(lights.size() > _lightBuffer.size())
        _lightBuffer.create(lights.size(), nullptr, DYNAMIC);

    int indexTexture = 0;
    vector<Std140LightData> data(lights.size());
    for(uint i=0 ; i<lights.size() ; ++i)
    {
        data[i].head = vec4(static_cast<float>(lights[i].type), lights[i].radius, lights[i].power, static_cast<float>(indexTexture));
        data[i].position = vec4(lights[i].position);
        data[i].color = lights[i].color;

        if(lights[i].type == Light::SPECULAR_PROB && lights[i].tex != nullptr)
            indexTexture ++;
    }

     _lightBuffer.flush(&data[0], 0, lights.size());
}

}
}
