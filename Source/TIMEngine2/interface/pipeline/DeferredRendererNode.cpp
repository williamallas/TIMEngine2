
#include "DeferredRendererNode.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
namespace pipeline
{

renderer::Texture* DeferredRendererNode::buffer(uint index) const
{
    if(!_rendererEntity) return nullptr;

    if(index == 0)
        return _rendererEntity->lightRenderer().buffer();
    else if(index-1 < 4) return _rendererEntity->deferredRenderer().buffer(index-1);
    else return nullptr;
}

void DeferredRendererNode::prepare()
{
    if(!tryPrepare()) return;

    _culledLight.clear();
    _toDraw.clear();

    for(size_t i=0 ; i<_meshInstanceSource.size() ; ++i)
    {
        if(!_meshInstanceSource[i]) continue;

        _meshInstanceSource[i]->prepare();
        const auto& culledMesh =_meshInstanceSource[i]->get();

        for(const MeshInstance& m : culledMesh)
        {
            for(uint i=0 ; i<m.mesh().nbElements() ; ++i)
                _toDraw.push_back({&(m.mesh().element(i)), &(m.matrix())});
        }
    }

    for(uint i=0 ; i<_dirLightDepthMapRenderer.size() ; ++i)
    {
        if(_dirLightDepthMapRenderer[i])
            _dirLightDepthMapRenderer[i]->prepare();
    }

    std::sort(_toDraw.begin(), _toDraw.end(), [](const ElementInstance& e1, const ElementInstance& e2)
        { return e1.first->drawState() < e2.first->drawState(); });

    for(size_t i=0 ; i<_lightInstanceSource.size() ; ++i)
    {
        if(!_lightInstanceSource[i]) continue;
        _lightInstanceSource[i]->prepare();
        const auto& vec = _lightInstanceSource[i]->get();
        _culledLight.insert(_culledLight.end(), vec.begin(), vec.end());
    }
}

void DeferredRendererNode::render()
{
    if(!tryRender()) return;

    if(_rendererEntity == nullptr)
        return;

    if(_sceneView)
        _meshDrawer.frameState().setCamera(_sceneView->camera);

    _rendererEntity->deferredRenderer().frameBuffer().bind();

    renderer::openGL.clearDepth();
    renderer::openGL.clearColor(vec4(0));

    if(!_toDraw.empty())
    {
        vector<mat4> accMatr;
        vector<renderer::MeshBuffers*> accMesh;
        vector<renderer::Material> accMate;
        renderer::DrawState curDrawState = _toDraw[0].first->drawState();
        uint curIndex=0;

        for(curIndex=0 ; curIndex < _toDraw.size() ; ++curIndex)
        {
            if(curDrawState != _toDraw[curIndex].first->drawState())
            {
                _meshDrawer.setDrawState(curDrawState);
                _meshDrawer.draw(accMesh, accMatr, accMate);
                accMesh.resize(0);
                accMatr.resize(0);
                accMate.resize(0);
                curDrawState = _toDraw[curIndex].first->drawState();
            }

            if(_toDraw[curIndex].first->geometry().buffers())
            {
                accMatr.push_back(_toDraw[curIndex].second->transposed());
                accMesh.push_back(_toDraw[curIndex].first->geometry().buffers());
                accMate.push_back(_toDraw[curIndex].first->internalMaterial());
            }
        }
        if(!accMesh.empty())
        {
            _meshDrawer.setDrawState(curDrawState);
            _meshDrawer.draw(accMesh, accMatr, accMate);
        }
    }

    _rendererEntity->lightRenderer().draw({});

    if(_globalLightInfo)
    {
        for(uint i=0 ; i<std::min(_dirLightDepthMapRenderer.size(),_globalLightInfo->dirLights.size()) ; ++i)
        {
            if(_dirLightDepthMapRenderer[i] && _globalLightInfo->dirLights[i].projectShadow)
                _dirLightDepthMapRenderer[i]->render();
        }

        vector<renderer::DirectionalLightRenderer::Light> lights(_globalLightInfo->dirLights.size());
        for(uint i=0 ; i<_globalLightInfo->dirLights.size() ; ++i)
        {
            lights[i].direction = _globalLightInfo->dirLights.at(i).direction;
            lights[i].color = vec3(_globalLightInfo->dirLights.at(i).color);
            lights[i].depthMap = nullptr;
            if(_globalLightInfo->dirLights.at(i).projectShadow && i<_dirLightDepthMapRenderer.size() &&
               _dirLightDepthMapRenderer[i])
            {
                lights[i].depthMap = _dirLightDepthMapRenderer[i]->buffer(0);
                lights[i].matrix =    _dirLightDepthMapRenderer[i]->matrix();
            }
        }
        _rendererEntity->dirLightRenderer().draw(lights);

        _rendererEntity->envLightRenderer().setSkybox(_globalLightInfo->skybox.first, _globalLightInfo->skybox.second);
        _rendererEntity->envLightRenderer().draw();
    }

    if(_rendererEntity->reflexionRenderer())
        _rendererEntity->reflexionRenderer()->draw();
}

void DeferredRendererNode::setRendererEntity(Pipeline::DeferredRendererEntity& entity)
{
    _rendererEntity = &entity;
}

}
}
}
