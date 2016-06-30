#include "FullPipeline.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace interface
{
using namespace interface::pipeline;

FullPipeline::~FullPipeline()
{
    delete pipeline;
}

void FullPipeline::setNull()
{
	hmdNode = nullptr;
	hmdEyeRendererNode[0] = nullptr;
	hmdEyeRendererNode[1] = nullptr;
	secondRendererEntity = nullptr;

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
	secondRendererEntity = nullptr;
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

    antialiasingFilter[0] = &pipeline->createNode<pipeline::SimpleFilter>();
    antialiasingFilter[0]->setShader(ShaderPool::instance().get("fxaa"));
    antialiasingFilter[0]->setBufferOutputNode(rendererNode->outputNode(0),0);

    onScreen->setBufferOutputNode(antialiasingFilter[0],0);
}

void FullPipeline::setScene(interface::Scene& scene, interface::View& view)
{
	if (rendererNode)
	{
		rendererNode->setGlobalLight(scene.globalLight);
		rendererNode->setSceneView(view);
	}
    
	if (meshCuller)
	{
		meshCuller->setScene(scene);
		meshCuller->setSceneView(view);
	}
    
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
    if(dirLightCuller)
        dirLightCuller->setSceneView(dirLightView);

    if(shadowRenderer)
        shadowRenderer->setSceneView(dirLightView);
}

}
}
#include "MemoryLoggerOff.h"
