#include "FullPipeline.h"

#include "MemoryLoggerOn.h"
namespace tim
{
using namespace interface;
using namespace interface::pipeline;

FullPipeline::~FullPipeline()
{
    delete pipeline;
}

void FullPipeline::setNull()
{
    pipeline=nullptr;
    rendererEntity=nullptr;
    onScreen=nullptr;
    rendererNode=nullptr;
    meshCuller=nullptr;
    lightCuller=nullptr;
    dirLightCuller=nullptr;
    shadowRenderer=nullptr;
}

void FullPipeline::create(uivec2 res, const Parameter& param)
{
    delete pipeline;
    setNull();

    pipeline = new interface::Pipeline;
    rendererEntity = &pipeline->genDeferredRendererEntity(res, param.usePointLight, param.useSSReflexion);
    onScreen = &pipeline->createNode<pipeline::OnScreenRenderer>();

    rendererNode = &pipeline->createNode<pipeline::DeferredRendererNode>();
    meshCuller = &pipeline->createNode<pipeline::SimpleSceneMeshCullingNode>();
    if(param.usePointLight) lightCuller = &pipeline->createNode<pipeline::SimpleSceneLightCullingNode>();
    if(param.useShadow) dirLightCuller = &pipeline->createNode<pipeline::DirLightCullingNode<SimpleScene>>();
    if(param.useShadow) shadowRenderer = &pipeline->createNode<pipeline::DirLightShadowNode>();

    pipeline->setOutputNode(*onScreen);
    rendererNode->setRendererEntity(*rendererEntity);

    if(param.useShadow)
    {
        shadowRenderer->setDepthMapResolution(param.shadowResolution);
        shadowRenderer->setShadowLightRange(param.shadowCascad);
        shadowRenderer->addMeshInstanceCollector(*dirLightCuller);
        dirLightCuller->setDepthMapResolution(param.shadowResolution);
        dirLightCuller->setShadowLightRange(param.shadowCascad);
        rendererNode->setDirLightShadow(*shadowRenderer, 0);
    }

    rendererNode->addMeshInstanceCollector(*meshCuller);
    if(param.usePointLight) rendererNode->addLightInstanceCollector(*lightCuller);
    onScreen->setBufferOutputNode(rendererNode->outputNode(0),0);
}

void FullPipeline::setScene(interface::Scene& scene, interface::View& view)
{
    if(!rendererNode) return;

    rendererNode->setGlobalLight(scene.globalLight);
    rendererNode->setSceneView(view);

    meshCuller->setScene(scene);
    meshCuller->setSceneView(view);

    if(lightCuller)
    {
        lightCuller->setScene(scene);
        lightCuller->setSceneView(view);
    }

    if(dirLightCuller)
        dirLightCuller->setScene(scene);
}

void FullPipeline::setDirLightView(interface::View& dirLightView)
{
    if(!rendererNode) return;

    if(dirLightCuller)
        dirLightCuller->setSceneView(dirLightView);

    if(shadowRenderer)
        shadowRenderer->setSceneView(dirLightView);
}

}
#include "MemoryLoggerOff.h"
