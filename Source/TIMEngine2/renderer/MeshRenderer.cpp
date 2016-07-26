#include "MeshRenderer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{


MeshRenderer::MeshRenderer()
#ifndef USE_VCPP
    : _maxUboMat4(openGL.hardward(GLState::Hardward::MAX_UNIFORM_BLOCK_SIZE) / (16*4))
#endif
{
#ifdef USE_SSBO_MODELS
    int *tmp = new int[1<<25];
    for(int i=0 ; i<(1<<25) ; ++i) tmp[i] = i;
    _drawIdBuffer.create(1<<25, tmp, VertexFormat::VEC1, DrawMode::STATIC, true);
    delete[] tmp;
#else
    int tmp[_maxUboMat4];
    for(uint i=0 ; i<_maxUboMat4 ; ++i) tmp[i] = static_cast<int>(i);
   _drawIdBuffer.create(_maxUboMat4, tmp, VertexFormat::VEC1, DrawMode::STATIC, true);
#endif
   _vao = new VAO(vertexBufferPool->buffer(), _drawIdBuffer);

   _modelBuffer.create(_maxUboMat4, nullptr, DrawMode::STREAM);
   _materialBuffer.create(_maxUboMat4, nullptr, DrawMode::STREAM);
}

MeshRenderer::~MeshRenderer()
{
    delete _vao;
}

void MeshRenderer::bind() const
{
    _vao->bind();
    indexBufferPool->buffer().bind();
}

void MeshRenderer::setDrawState(const DrawState& s)
{
    _states = s;
}

int MeshRenderer::draw(const vector<MeshBuffers*>& meshs, const vector<mat4>& models, const vector<DummyMaterial>& materials,
                       const vector<vector<uint>>& extraUbo, bool useCameraUbo)
{
    if(meshs.empty() || models.size() != meshs.size() || (!materials.empty() && materials.size() < meshs.size())
       || (!extraUbo.empty() && extraUbo.size() < meshs.size()))
        return 0;

    openGL.alphaTest(false);

#ifdef USE_SSBO_MODELS
    if(_modelBuffer.size() < models.size())
        _modelBuffer.create(models.size(), &models[0], DrawMode::STREAM);
    else
        _modelBuffer.flush(&models[0], 0, models.size());

    openGL.bindShaderStorageBuffer(_modelBuffer.id(), 1);
#endif

    _states.bind();
    bind();

#ifndef USE_SSBO_MODELS
    uint nbLoop = models.size() / _maxUboMat4;
    if(models.size()%_maxUboMat4 > 0) nbLoop++;

    IndirectDrawParmeter drawParam[_maxUboMat4];
    for(uint i=0 ; i<nbLoop ; ++i)
    {
        uint innerLoop = std::min(_maxUboMat4,models.size() - i*_maxUboMat4);
        _modelBuffer.flush(&models[_maxUboMat4*i], 0, innerLoop);

        if(!materials.empty())
        {
            _materialBuffer.flush(&materials[_maxUboMat4*i], 0, innerLoop);
        }

        for(uint j=0 ; j<innerLoop ; ++j)
        {
            drawParam[j].count = meshs[_maxUboMat4*i+j]->ib()->size();
            drawParam[j].baseInstance = j;
            drawParam[j].baseVertex = meshs[_maxUboMat4*i+j]->vb()->offset();
            drawParam[j].instanceCount = 1;
            drawParam[j].firstIndex = meshs[_maxUboMat4*i+j]->ib()->offset();
        }

        if(useCameraUbo)
            _parameter.bind(0);

        openGL.bindUniformBuffer(_modelBuffer.id(), 1);

        if(!extraUbo.empty())
        {
            for(uint j=0 ; j<extraUbo[i].size() ; ++j)
                openGL.bindUniformBuffer(extraUbo[i][j], 3+j);
        }

        if(!materials.empty())
            openGL.bindUniformBuffer(_materialBuffer.id(), 2);

        glMultiDrawElementsIndirect(DrawState::toGLPrimitive(_states.primitive()), GL_UNSIGNED_INT, drawParam, innerLoop, 0);
    }
#else
    openGL.bindUniformBuffer(_uboParameter.id(), 0);
    openGL.bindShaderStorageBuffer(_modelBuffer.id(), 1);

    glMultiDrawElementsIndirect(DrawState::toGLPrimitive(_states.primitive()), GL_UNSIGNED_INT, drawParam, 1, 0);
#endif

    return 0;
}

}
}
