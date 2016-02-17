#include "MeshRenderer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

MeshRenderer::MeshRenderer() : _maxUboMa4(openGL.hardward(GLState::Hardward::MAX_UNIFORM_BLOCK_SIZE) / (16*4))
{
    _parameter.proj = mat4::Projection(70,1,1,100);
    _uboParameter.create(1, &_parameter, DrawMode::STREAM);
    _hasChanged = false;

#ifdef USE_SSBO_MODELS
    int *tmp = new int[1<<25];
    for(int i=0 ; i<(1<<25) ; ++i) tmp[i] = i;
    _drawIdBuffer.create(1<<25, tmp, VertexFormat::VEC1, DrawMode::STATIC, true);
    delete[] tmp;
#else
    int tmp[_maxUboMa4];
    for(int i=0 ; i<_maxUboMa4 ; ++i) tmp[i] = i;
   _drawIdBuffer.create(_maxUboMa4, tmp, VertexFormat::VEC1, DrawMode::STATIC, true);
#endif
   _vao = new VAO(vertexBufferPool->buffer(), _drawIdBuffer);

   _modelBuffer.create(_maxUboMa4, nullptr, DrawMode::STREAM);
   _materialBuffer.create(_maxUboMa4, nullptr, DrawMode::STREAM);
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

void MeshRenderer::setCamera(const Camera& camera)
{
    _parameter.proj = mat4::Projection(camera.fov, camera.ratio, camera.clipDist.x(), camera.clipDist.y());
    _parameter.view = mat4::View(camera.pos, camera.dir, camera.up);
    _parameter.cameraPos = vec4(camera.pos,0);
    _parameter.cameraDir = vec4(camera.dir,0);
    _parameter.cameraUp = vec4(camera.up,0);
    _hasChanged = true;
}

void MeshRenderer::setViewMatrix(const mat4& view, const vec3& camPos)
{
    _parameter.view = view;
    _parameter.cameraPos = vec4(camPos,0);
    _hasChanged = true;
}

void MeshRenderer::setWordlOrigin(const vec3& wo)
{
    _parameter.worldOrigin = vec4(wo,0);
    _hasChanged = true;
}

void MeshRenderer::setProjectionMatrix(const mat4& proj)
{
    _parameter.proj = proj;
    _hasChanged = true;
}

void MeshRenderer::setTime(float time, float frameTime)
{
    _parameter.time = {time, frameTime,0,0 };
    _hasChanged = true;
}

void MeshRenderer::updateParameter()
{
    _parameter.invProj = _parameter.proj.inverted();
    _parameter.invView = _parameter.view.inverted();
    _parameter.projView = _parameter.proj * _parameter.view;
    _parameter.invViewProj = _parameter.invView * _parameter.invProj;

    _parameter.worldOriginInvViewProj = _parameter.view;
    _parameter.worldOriginInvViewProj[0].w()=0;
    _parameter.worldOriginInvViewProj[1].w()=0;
    _parameter.worldOriginInvViewProj[2].w()=0;
    _parameter.worldOriginInvViewProj *= mat4::Translation(-_parameter.cameraPos.to<3>()+_parameter.worldOrigin.to<3>());
    _parameter.worldOriginInvViewProj = (_parameter.proj*_parameter.worldOriginInvViewProj).inverted();
    _hasChanged = false;

    Parameter tmp = _parameter;
    tmp.invProj.transpose();
    tmp.invView.transpose();
    tmp.invViewProj.transpose();
    tmp.projView.transpose();
    tmp.proj.transpose();
    tmp.view.transpose();
    tmp.worldOriginInvViewProj.transpose();
    _uboParameter.flush(&tmp, 0,1);
}

int MeshRenderer::draw(const vector<MeshBuffers*>& meshs, const vector<mat4>& models, const vector<Material>& materials)
{
    if(meshs.empty() || models.size() != meshs.size() || (!materials.empty() && materials.size() < meshs.empty()))
        return 0;

    if(_hasChanged) updateParameter();

#ifdef USE_SSBO_MODELS
    if(_modelBuffer.size() < models.size())
        _modelBuffer.create(models.size(), &models[0], DrawMode::STREAM);
    else
        _modelBuffer.flush(&models[0], 0, models.size());

    openGL.bindShaderStorageBuffer(_modelBuffer.id(), 1);
#endif

    _states.bind();
    _vao->bind();
    indexBufferPool->buffer().bind();

#ifndef USE_SSBO_MODELS
    uint nbLoop = models.size() / _maxUboMa4;
    if(models.size()%_maxUboMa4 > 0) nbLoop++;

    IndirectDrawParmeter drawParam[_maxUboMa4];
    for(uint i=0 ; i<nbLoop ; ++i)
    {
        uint innerLoop = std::min(_maxUboMa4,models.size() - i*_maxUboMa4);
        _modelBuffer.flush(&models[_maxUboMa4*i], 0, innerLoop);

        if(!materials.empty())
            _materialBuffer.flush(&materials[_maxUboMa4*i], 0, innerLoop);

        for(uint j=0 ; j<innerLoop ; ++j)
        {
            drawParam[j].count = meshs[_maxUboMa4*i+j]->ib()->size();
            drawParam[j].baseInstance = j;
            drawParam[j].baseVertex = meshs[_maxUboMa4*i+j]->vb()->offset();
            drawParam[j].instanceCount = 1;
            drawParam[j].firstIndex = meshs[_maxUboMa4*i+j]->ib()->offset();
        }

        openGL.bindUniformBuffer(_uboParameter.id(), 0);
        openGL.bindUniformBuffer(_modelBuffer.id(), 1);

        if(!materials.empty())
            openGL.bindUniformBuffer(_materialBuffer.id(), 2);

        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, drawParam, innerLoop, 0);
    }
#else
    openGL.bindUniformBuffer(_uboParameter.id(), 0);
    openGL.bindShaderStorageBuffer(_modelBuffer.id(), 1);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, drawParam, 1, 0);
#endif

    return 0;
}

}
}
