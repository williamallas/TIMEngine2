#include "Pipeline.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{

Pipeline::DeferredRendererEntity& Pipeline::genDeferredRendererEntity(const uivec2& res, bool useLightRenderer, bool useReflexionRenderer, int entityId)
{
    auto it = _deferredRendererEntity.find(boost::make_tuple(res, useLightRenderer, useReflexionRenderer, entityId));
    if(it != _deferredRendererEntity.end())
        return *(it->second.get());
    else
    {
        DeferredRendererEntity* entity = new DeferredRendererEntity(res, _meshRenderer.frameState(), useLightRenderer, useReflexionRenderer);
        _deferredRendererEntity[boost::make_tuple(res, useLightRenderer, useReflexionRenderer, entityId)]
                = std::unique_ptr<DeferredRendererEntity>(entity);

        return *entity;
    }
}

Pipeline::Pipeline()
{
    _meshRenderer.bind();
}

Pipeline::~Pipeline()
{
    for(uint i=0 ; i<_allProcessNodes.size() ; ++i)
    {
        delete _allProcessNodes[i];
    }
}

renderer::MeshRenderer& Pipeline::meshRenderer()
{
    return _meshRenderer;
}

void Pipeline::setOutputNode(Pipeline::TerminalNode& node)
{
    _outputNode = &node;
}

void Pipeline::prepare()
{
    for(uint i=0 ; i<_allProcessNodes.size() ; ++i)
        _allProcessNodes[i]->reset();

    if(_outputNode)
        _outputNode->prepare();
}

void Pipeline::render()
{
    if(_outputNode)
        _outputNode->render();

    renderer::openGL.execAllGLTask();
}

void Pipeline::SceneView::offset(vec3 o)
{
    if(camera.useRawMat)
        camera.raw_view = camera.raw_view * mat4::Translation(-o);

    camera.dir += o;
    camera.pos += o;

    dirLightView.camPos += o;
}

void Pipeline::SceneView::offset(const mat4& o, const mat4& inv_o)
{
    if(camera.useRawMat)
        camera.raw_view = camera.raw_view * inv_o;

    camera.dir = o * camera.dir;
    camera.pos = o * camera.pos;

    dirLightView.camPos = o * dirLightView.camPos;
}

}
}
