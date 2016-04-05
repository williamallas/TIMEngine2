#include "DirectionalLightRenderer.h"
#include "ShaderCompiler.h"
#include "renderer.h"
#include "MeshBuffers.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

DirectionalLightRenderer::DirectionalLightRenderer(LightContextRenderer& context, Shader* shader) : _context(context), _shader(shader)
{
    if(!_shader)
    {
        ShaderCompiler vShader(ShaderCompiler::VERTEX_SHADER), pShader(ShaderCompiler::PIXEL_SHADER);
        vShader.setSource(StringUtils::readFile("shader/directionalLight.vert"));
        pShader.setSource(StringUtils::readFile("shader/directionalLight.frag"));

        auto optShader = Shader::combine(vShader.compile({}), pShader.compile({}));

        if(!optShader.hasValue())
        {
            LOG("shader/directionalLight.vert error:\n", vShader.error());
            LOG("shader/directionalLight.frag error:\n", pShader.error());
            LOG("Link erorr:", Shader::lastLinkError());
        }
        else
            _shader = optShader.value();
    }

    _shader->bind();
    _uniformNbLight = _shader->uniformLocation("nbLights");
    _uniformLightColor = _shader->uniformLocation("lightColor");
    _uniformLightDir = _shader->uniformLocation("lightDirection");
    _uniformLightMatrix = _shader->uniformLocation("lightMatrix");

    _state.setBlend(true);
    _state.setBlendEquation(DrawState::BlendEquation::ADD);
    _state.setBlendFunc({DrawState::BlendFunc::ONE, DrawState::BlendFunc::ONE});
    _state.setCullFace(false);
    _state.setWriteDepth(false);
    _state.setDepthTest(false);
    _state.setShader(_shader);
}

DirectionalLightRenderer::~DirectionalLightRenderer()
{
    delete _shader;
}

void DirectionalLightRenderer::draw(const vector<Light>& lights) const
{
    _context.frameBuffer().bind();
    _state.bind();

    int nbLight = static_cast<int>(lights.size());
    _shader->setUniform(nbLight, _uniformNbLight);

    for(int i=0 ; i<4 ; ++i)
    {
        openGL.bindTextureSampler(textureSampler[TextureMode::NoFilter], i);
        _context.deferred().buffer(i)->bind(i);
    }

    if(nbLight > 0)
    {
        std::unique_ptr<vec4[]> lightDir(new vec4[nbLight]);
        std::unique_ptr<vec4[]> lightColor(new vec4[nbLight]);
        std::unique_ptr<mat4[]> lightMatrix(new mat4[nbLight*MAX_SHADOW_MAP_LVL]);

        for(size_t i=0 ; i<lights.size() ; ++i)
        {
            lightDir[i] = vec4(lights[i].direction,0);
            lightColor[i] = vec4(lights[i].color,0);

            if(lights[i].depthMap && !lights[i].matrix.empty())
            {
                openGL.bindTextureSampler(textureSampler[TextureMode::DepthMap], 4+i);
                lights[i].depthMap->bind(4+i);

                lightDir[i].w() = static_cast<float>(lights[i].matrix.size());
                for(size_t j=0 ; j<lights[i].matrix.size() ; ++j)
                    lightMatrix[i*MAX_SHADOW_MAP_LVL+j] = lights[i].matrix[j];
            }
        }

        _shader->setUniform(&lightDir[0], lights.size(), _uniformLightDir);
        _shader->setUniform(&lightColor[0], lights.size(), _uniformLightColor);
        _shader->setUniform(&lightMatrix[0], lights.size()*MAX_SHADOW_MAP_LVL, _uniformLightMatrix);
    }

    _context.frameState().bind(0);
    quadMeshBuffers->draw(6, VertexMode::TRIANGLES, 1);
}

}
}
