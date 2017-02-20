#include "IndirectLightRenderer.h"

#include "ShaderCompiler.h"
#include "renderer.h"
#include "MeshBuffers.h"
#include "core/Rand.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

IndirectLightRenderer::IndirectLightRenderer(LightContextRenderer& context) : _context(context)
{
    if(!_fullScreenPass)
    {
        ShaderCompiler vShader(ShaderCompiler::VERTEX_SHADER), pShader(ShaderCompiler::PIXEL_SHADER);
        vShader.setSource(StringUtils::readFile("shader/globalIndirectLight.vert"));
        pShader.setSource(StringUtils::readFile("shader/globalIndirectLight.frag"));

        auto optShader = Shader::combine(vShader.compile({}), pShader.compile({}));

        if(!optShader.hasValue())
        {
            LOG("shader/globalIndirectLight.vert error:\n", vShader.error());
            LOG("shader/globalIndirectLight.frag error:\n", pShader.error());
            LOG("Link erorr:", Shader::lastLinkError());
        }
        else
        {
            _fullScreenPass = optShader.value();
            _fullScreenPass->bind();
            _uniformEnableGI = _fullScreenPass->uniformLocation("enableGI");
            _uniformGlobalAmbient = _fullScreenPass->uniformLocation("globalAmbient");
            _uniformSSReflexion = _fullScreenPass->uniformLocation("localReflexion");

            _uniformNbLight = _fullScreenPass->uniformLocation("nbLights");
            _uniformLightColor = _fullScreenPass->uniformLocation("lightColor");
            _uniformLightDir = _fullScreenPass->uniformLocation("lightDirection");
            _uniformLightMatrix = _fullScreenPass->uniformLocation("lightMatrix");
        }
    }

    Texture::GenTexParam param;
    param.format = Texture::RGB16;
    param.nbLevels = 1;
    param.size = uivec3(256,256,1);
    float* dat = new float[256*256*3];
    std::ifstream inDat("shader/brdf_256.dat", std::ios_base::binary);
    if(!inDat)
    {
        LOG("shader/brdf_256.dat does not exist.");
        exit(-1);
    }

    inDat.read((char*)dat, sizeof(float)*256*256*3);
    _processedBrdf = Texture::genTexture2D(param, dat, 3);
    delete[] dat;

    _stateFullScreenPass.setBlend(true);
    _stateFullScreenPass.setBlendEquation(DrawState::BlendEquation::ADD);
    _stateFullScreenPass.setBlendFunc({DrawState::BlendFunc::ONE, DrawState::BlendFunc::ONE});
    _stateFullScreenPass.setCullFace(false);
    _stateFullScreenPass.setWriteDepth(false);
    _stateFullScreenPass.setDepthTest(false);
    _stateFullScreenPass.setShader(_fullScreenPass);
}

IndirectLightRenderer::~IndirectLightRenderer()
{
    delete _fullScreenPass;
    delete _processedBrdf;
}

void IndirectLightRenderer::draw(const vector<Light>& lights) const
{
    _context.frameBuffer()->bind();
    _stateFullScreenPass.bind();

    for(int i=0 ; i<4 ; ++i)
    {
        _context.deferred().buffer(i)->bind(i);
        openGL.bindTextureSampler(textureSampler[TextureMode::NoFilter], i);
    }

    if(_skybox)
    {
        _skybox->bind(4);
        openGL.bindTextureSampler(textureSampler[TextureMode::FilteredNoRepeat], 4);
    } else openGL.bindTexture(0, GL_TEXTURE_CUBE_MAP, 4);

    if(_processedSkybox)
    {
        _processedSkybox->bind(5);
        openGL.bindTextureSampler(textureSampler[TextureMode::FilteredNoRepeat], 5);
    } else openGL.bindTexture(0, GL_TEXTURE_CUBE_MAP, 5);

    if(_processedBrdf)
    {
        _processedBrdf->bind(6);
        openGL.bindTextureSampler(textureSampler[TextureMode::FilteredNoRepeat], 6);
    } else openGL.bindTexture(0, GL_TEXTURE_2D, 6);

    if(_inReflexionBuffer && _enableSSReflexion)
    {
        _inReflexionBuffer->bind(7);
        openGL.bindTextureSampler(textureSampler[TextureMode::FilteredNoRepeat], 7);
    }

    int nbLight = static_cast<int>(lights.size());
    _fullScreenPass->setUniform(nbLight, _uniformNbLight);

    if(nbLight > 0)
    {
        std::unique_ptr<vec4[]> lightDir(new vec4[nbLight]);
        std::unique_ptr<vec4[]> lightColor(new vec4[nbLight]);
        std::unique_ptr<mat4[]> lightMatrix(new mat4[MAX_SHADOW_MAP_LVL]);

        for(size_t i=0 ; i<lights.size() ; ++i)
        {
            lightDir[i] = vec4(lights[i].direction,0);
            lightColor[i] = vec4(lights[i].color,0);

            if(lights[i].depthMap && !lights[i].matrix.empty())
            {
                openGL.bindTextureSampler(textureSampler[TextureMode::DepthMap], 8);
                lights[i].depthMap->bind(8);

                lightDir[i].w() = static_cast<float>(lights[i].matrix.size());
                for(size_t j=0 ; j<lights[i].matrix.size() ; ++j)
                    lightMatrix[j] = lights[i].matrix[j];
            }
        }

        _fullScreenPass->setUniform(&lightDir[0], lights.size(), _uniformLightDir);
        _fullScreenPass->setUniform(&lightColor[0], lights.size(), _uniformLightColor);
        _fullScreenPass->setUniform(&lightMatrix[0], MAX_SHADOW_MAP_LVL, _uniformLightMatrix);
    }

    _fullScreenPass->setUniform((_enableGI && _processedSkybox)?1:0, _uniformEnableGI);
    _fullScreenPass->setUniform(_enableSSReflexion?1:0, _uniformSSReflexion);
    _fullScreenPass->setUniform(_globalAmbient, _uniformGlobalAmbient);

    _context.frameState().bind(0);
    quadMeshBuffers->draw(6, VertexMode::TRIANGLES, 1);
}

Texture* IndirectLightRenderer::processSkybox(Texture* tex, Shader* shader)
{
    if(!tex)
        return nullptr;

    DrawState state;
    state.setBlend(false);
    state.setCullFace(false);
    state.setWriteDepth(false);
    state.setDepthTest(false);
    state.setShader(shader);

    Texture::GenTexParam param;
    param.format = Texture::Format::RGBA8;
    param.nbLevels = NB_MIPMAP;
    param.size = uivec3(MAX_RESOLUTION,MAX_RESOLUTION,1);
    Texture* processedSkybox = Texture::genTextureCube(param);


    FrameBuffer fbo;
    int roughnessId = shader->uniformLocation("inRoughness");

    state.bind();
    openGL.bindTextureSampler(textureSampler[TextureMode::FilteredNoRepeat], 0);
    tex->bind(0);
    openGL.bindFrameBuffer(0);

    float rough[NB_MIPMAP] = {0, 0.05, 0.13, 0.25, 0.45, 0.66, 1};

    for(uint i=0 ; i<NB_MIPMAP ; ++i)
    {
        fbo.setResolution({MAX_RESOLUTION >> i,MAX_RESOLUTION >> i});
        fbo.bind();
        for(int j=0 ; j<6 ; ++j)
            fbo.attachTexture(j, processedSkybox, i, j);

        state.shader()->setUniform(rough[i], roughnessId);
        quadMeshBuffers->draw(6, VertexMode::TRIANGLES, 1);
    }

    return processedSkybox;
}

vec3 importanceSampleGGX(vec2 Xi, float roughness, vec3 N)
{
    float a = roughness * roughness;
    float phi = 2 * PI * Xi.x();
    float cosTheta = sqrt((1 - Xi.y()) / (1 + (a*a - 1) * Xi.y()));
    float sinTheta = sqrt(1 - cosTheta * cosTheta);
    vec3 H = vec3(sinTheta * cos( phi ), sinTheta * sin( phi ), cosTheta);

    vec3 upVector = abs(N.z()) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
    vec3 tangentX = upVector.cross(N).normalized();
    vec3 tangentY = N.cross(tangentX);

    return tangentX * H.x() + tangentY * H.y() + N * H.z();
}

float helper_G_smith(float dotVal, float k)
{
    return dotVal / (dotVal*(1-k)+k);
}

float G_Smith(float dotNV, float dotNL, float roughness)
{
    float k = roughness*roughness / 2;
    return helper_G_smith(dotNL,k)*helper_G_smith(dotNV,k);
}

vec2 integrateBRDF(float Roughness, float NoV)
{
    vec3 V;
    V.x() = sqrt( 1.0f - NoV * NoV ); // sin
    V.y() = 0;
    V.z() = NoV; // cos
    float A = 0;
    float B = 0;
    const uint NumSamples = 1<<16;
    for( uint i = 0; i < NumSamples; i++ )
    {
        vec2 Xi = vec2(Rand::frand(), Rand::frand());
        vec3 H = importanceSampleGGX(Xi, Roughness, vec3(0,0,1));
        vec3 L =  H * V.dot(H) * 2 - V;
        float NoL = std::max(L.z(),0.f);
        float NoH = std::max(H.z(),0.f);
        float VoH = std::max(V.dot(H), 0.f);

        if(NoL > 0)
        {
            float G = G_Smith(NoV, NoL, Roughness);
            float G_Vis = G * VoH / (NoH * NoV);

            float Fc = pow(1 - VoH, 5);
            A += (1 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    return vec2(A, B) / NumSamples;
}

float* IndirectLightRenderer::computeBrdf(uint size)
{
    resource::Image img({size,size});
    float* res = new float[size*size*3];

    for(uint i=0 ; i<size ; ++i)
    {
        for(uint j=0 ; j<size ; ++j)
        {
            vec2 v = integrateBRDF(float(i)/(size-1), float(j+1)/size);
            res[(i*size+j)*3] = v.x();
            res[(i*size+j)*3+1] = v.y();
            res[(i*size+j)*3+2] = 0;
            img.setPixel(ubvec3(v.x()*255, v.y()*255,0), {i,j});
        }
    }

    img.exportBmp("brdf.bmp");
    return res;
}

std::pair<Texture*, Texture*> IndirectLightRenderer::loadAndProcessSkybox(const vector<std::string>& files, Shader* processShader)
{
    if(files.size() != 6)
        return {nullptr, nullptr};

    resource::TextureLoader::ImageFormat formatTex;
    vector<ubyte*> dataSkybox = resource::textureLoader->loadImageCube(files, formatTex);

    if(dataSkybox.size() < 6)
        return {nullptr, nullptr};

    renderer::Texture::GenTexParam skyboxParam;
    skyboxParam.format = renderer::Texture::RGBA8;
    skyboxParam.nbLevels = 1;
    skyboxParam.size = uivec3(formatTex.size,0);
    renderer::Texture* skybox = renderer::Texture::genTextureCube(skyboxParam, dataSkybox, 4);
    renderer::Texture* processedSky = renderer::IndirectLightRenderer::processSkybox(skybox, processShader);

    for(uint j=0 ; j<dataSkybox.size() ; ++j)
        delete[] dataSkybox[j];

    return {skybox, processedSky};
}

}
}

