#include "FullPipeline.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace interface
{
using namespace interface::pipeline;

FullPipeline::~FullPipeline()
{
    delete _pipeline;
}

Pipeline::InOutBufferNode* FullPipeline::combineNode(int index) const
{
    return _combineMultipleScene[index];
}

const vector<pipeline::DeferredRendererNode*>& FullPipeline::deferredRendererNode(int chanel, int eye) const
{
    return _deferredRendererNodes[eye][chanel];
}

void FullPipeline::setScene(Scene& scene, int sceneId)
{
    for(size_t i=0 ; i<_deferredRendererNodes[0][sceneId].size() ; ++i)
    {
        if(_deferredRendererNodes[0][sceneId][i])
            _deferredRendererNodes[0][sceneId][i]->setGlobalLight(scene.globalLight);
    }

    for(size_t i=0 ; i<_deferredRendererNodes[1][sceneId].size() ; ++i)
    {
        if(_deferredRendererNodes[1][sceneId][i])
            _deferredRendererNodes[1][sceneId][i]->setGlobalLight(scene.globalLight);
    }

    for(size_t i=0 ; i<_meshCullingNodes[sceneId].size() ; ++i)
    {
        if(_meshCullingNodes[sceneId][i])
            _meshCullingNodes[sceneId][i]->setScene(scene);
    }

    for(size_t i=0 ; i<_lightCullingNodes[sceneId].size() ; ++i)
    {
        if(_lightCullingNodes[sceneId][i])
            _lightCullingNodes[sceneId][i]->setScene(scene);
    }

    for(size_t i=0 ; i<_dirLightCullingNodes[sceneId].size() ; ++i)
    {
        if(_dirLightCullingNodes[sceneId][i])
            _dirLightCullingNodes[sceneId][i]->setScene(scene);
    }
}

void FullPipeline::setScene(interface::Scene& scene, interface::View& view, int sceneId)
{
    setScene(scene, sceneId);

    for(size_t i=0 ; i<_deferredRendererNodes[0][sceneId].size() ; ++i)
    {
        if(_deferredRendererNodes[0][sceneId][i])
            _deferredRendererNodes[0][sceneId][i]->setSceneView(view);
    }

    for(size_t i=0 ; i<_meshCullingNodes[sceneId].size() ; ++i)
    {
        if(_meshCullingNodes[sceneId][i])
            _meshCullingNodes[sceneId][i]->setSceneView(view);
    }

    for(size_t i=0 ; i<_lightCullingNodes[sceneId].size() ; ++i)
    {
        if(_lightCullingNodes[sceneId][i])
            _lightCullingNodes[sceneId][i]->setSceneView(view);
    }
}

void FullPipeline::setStereoView(View& cullingView, View& eye1,  View& eye2, int sceneId)
{
    for(size_t i=0 ; i<_deferredRendererNodes[0][sceneId].size() ; ++i)
    {
        if(_deferredRendererNodes[0][sceneId][i])
            _deferredRendererNodes[0][sceneId][i]->setSceneView(eye1);
    }

    for(size_t i=0 ; i<_deferredRendererNodes[1][sceneId].size() ; ++i)
    {
        if(_deferredRendererNodes[1][sceneId][i])
            _deferredRendererNodes[1][sceneId][i]->setSceneView(eye2);
    }

    for(size_t i=0 ; i<_meshCullingNodes[sceneId].size() ; ++i)
    {
        if(_meshCullingNodes[sceneId][i])
            _meshCullingNodes[sceneId][i]->setSceneView(cullingView);
    }

    for(size_t i=0 ; i<_lightCullingNodes[sceneId].size() ; ++i)
    {
        if(_lightCullingNodes[sceneId][i])
            _lightCullingNodes[sceneId][i]->setSceneView(cullingView);
    }
}

void FullPipeline::setDirLightView(interface::View& dirLightView, int sceneId)
{
    for(size_t i=0 ; i<_dirLightCullingNodes[sceneId].size() ; ++i)
    {
        if(_dirLightCullingNodes[sceneId][i])
            _dirLightCullingNodes[sceneId][i]->setSceneView(dirLightView);
    }

    for(size_t i=0 ; i<_shadowMapNodes[sceneId].size() ; ++i)
    {
        if(_shadowMapNodes[sceneId][i])
            _shadowMapNodes[sceneId][i]->setSceneView(dirLightView);
    }
}

void FullPipeline::create(uivec2 res, const Parameter& param)
{
    delete _pipeline;
    setNull();

    _pipeline = new interface::Pipeline;
    _stereoscopy = false;

    pipeline::OnScreenRenderer& onScreen = _pipeline->createNode<pipeline::OnScreenRenderer>();

    if(param.useFxaa)
    {
        pipeline::SimpleFilter& antialiasNode = _pipeline->createNode<pipeline::SimpleFilter>();
        antialiasNode.setShader(ShaderPool::instance().get("fxaa"));
        antialiasNode.setBufferOutputNode(createSubDeferredPipeline(res, param, 0)->outputNode(0),0);
        onScreen.setBufferOutputNode(&antialiasNode,0);
    }
    else
    {
        onScreen.setBufferOutputNode(createSubDeferredPipeline(res, param, 0)->outputNode(0),0);
    }

    _pipeline->setOutputNode(onScreen);
}

void FullPipeline::createTwoScene(uivec2 res, const Parameter& param1, const Parameter& param2)
{
    createExtensible(res, param1);
    extendPipeline(res, param2, 1);
}

void FullPipeline::createExtensible(uivec2 res, const Parameter& param)
{
    delete _pipeline;
    setNull();

    _pipeline = new interface::Pipeline;
    _stereoscopy = false;

    pipeline::OnScreenRenderer& onScreen = _pipeline->createNode<pipeline::OnScreenRenderer>();

    pipeline::SimpleFilter& copyNode = _pipeline->createNode<pipeline::SimpleFilter>();
    copyNode.setShader(renderer::drawQuadShader);
    auto pipeline = createSubDeferredPipeline(res, param, 0);
    copyNode.setBufferOutputNode(pipeline->outputNode(0),0);

    pipeline::SimpleFilter& copyMaterial = _pipeline->createNode<pipeline::SimpleFilter>();
    copyMaterial.setShader(renderer::drawQuadShader);
    copyMaterial.setBufferOutputNode(pipeline->outputNode(3),0);

    pipeline::SimpleFilter& combineNode = _pipeline->createNode<pipeline::SimpleFilter>();
    combineNode.setShader(ShaderPool::instance().get("combineScene"));

    combineNode.setBufferOutputNode(&copyMaterial, 0);
    combineNode.setBufferOutputNode(&copyNode, 1);

    _combineMultipleScene[0] = &combineNode;

    if(param.useFxaa)
    {
        pipeline::SimpleFilter& antialiasNode = _pipeline->createNode<pipeline::SimpleFilter>();
        antialiasNode.setShader(ShaderPool::instance().get("fxaa"));
        antialiasNode.setBufferOutputNode(&combineNode, 0);
        onScreen.setBufferOutputNode(&antialiasNode, 0);
    }
    else
    {
        onScreen.setBufferOutputNode(&combineNode, 0);
    }

    _pipeline->setOutputNode(onScreen);
}

void FullPipeline::extendPipeline(uivec2 res, const Parameter& param, int index)
{
    if(_combineMultipleScene[0] == nullptr || (_stereoscopy && _combineMultipleScene[1] == nullptr))
        return;

    if(_stereoscopy)
    {
        auto stereoPipeline = createSubStereoDeferredPipeline(res, param, index);
//        pipeline::SimpleFilter& copyNode1 = _pipeline->createNode<pipeline::SimpleFilter>();
//        pipeline::SimpleFilter& copyNode2 = _pipeline->createNode<pipeline::SimpleFilter>();

//        copyNode1.setShader(renderer::drawQuadShader);
//        copyNode1.setBufferOutputNode(stereoPipeline.first->outputNode(0), 0);

//        copyNode2.setShader(renderer::drawQuadShader);
//        copyNode2.setBufferOutputNode(stereoPipeline.second->outputNode(0), 0);

        _combineMultipleScene[0]->setBufferOutputNode(stereoPipeline.first->outputNode(0), index+1);
        _combineMultipleScene[1]->setBufferOutputNode(stereoPipeline.second->outputNode(0), index+1);
    }
    else
    {
        pipeline::SimpleFilter& copyNode = _pipeline->createNode<pipeline::SimpleFilter>();
        copyNode.setShader(renderer::drawQuadShader);
        copyNode.setBufferOutputNode(createSubDeferredPipeline(res, param, index)->outputNode(0), 0);

        _combineMultipleScene[0]->setBufferOutputNode(&copyNode, index+1);
    }
}

void FullPipeline::createStereo(Pipeline::TerminalNode& hmdNode, uivec2 res, const Parameter& param)
{
    delete _pipeline;
    setNull();

    _pipeline = new interface::Pipeline;
    _stereoscopy = true;

    _pipeline->registerNode(&hmdNode);

    auto stereoRenderer = createSubStereoDeferredPipeline(res, param, 0);

    if(param.useFxaa)
    {
        pipeline::SimpleFilter* antialiasNode[2];

        for(int i=0 ; i<2 ; ++i)
        {
            antialiasNode[i] = &_pipeline->createNode<pipeline::SimpleFilter>();
            antialiasNode[i]->setShader(ShaderPool::instance().get("fxaa"));
            antialiasNode[i]->setBufferOutputNode((i==0?stereoRenderer.first:stereoRenderer.second)->outputNode(0), 0);
            hmdNode.setBufferOutputNode(antialiasNode[i], i);
        }
    }
    else
    {
        hmdNode.setBufferOutputNode(stereoRenderer.first->outputNode(0), 0);
        hmdNode.setBufferOutputNode(stereoRenderer.second->outputNode(0), 1);
    }

    _pipeline->setOutputNode(hmdNode);
}

void FullPipeline::createStereoExtensible(Pipeline::TerminalNode& hmdNode, uivec2 res, const Parameter& param)
{
    delete _pipeline;
    setNull();

    _pipeline = new interface::Pipeline;
    _stereoscopy = true;

    _pipeline->registerNode(&hmdNode);

    auto stereoRenderer = createSubStereoDeferredPipeline(res, param, 0);

    Pipeline::OutBuffersNode* stereo[2] = {stereoRenderer.first, stereoRenderer.second};

    pipeline::SimpleFilter *combineNode[2];
    //pipeline::SimpleFilter *copyNode[2], *copyMaterial[2];

    for(int i=0 ; i<2 ; ++i)
    {
        //copyNode[i] = &_pipeline->createNode<pipeline::SimpleFilter>();
        //copyMaterial[i] = &_pipeline->createNode<pipeline::SimpleFilter>();
        combineNode[i] = &_pipeline->createNode<pipeline::SimpleFilter>();

//        copyNode[i]->setShader(renderer::drawQuadShader);
//        copyNode[i]->setBufferOutputNode(stereo[i]->outputNode(0),0);

//        copyMaterial[i]->setShader(renderer::drawQuadShader);
//        copyMaterial[i]->setBufferOutputNode(stereo[i]->outputNode(3),0);

//        combineNode[i]->setShader(ShaderPool::instance().get("combineScene"));
//        combineNode[i]->setBufferOutputNode(copyMaterial[i], 0);
//        combineNode[i]->setBufferOutputNode(copyNode[i], 1);

        combineNode[i]->setShader(ShaderPool::instance().get("combineScene"));
        combineNode[i]->setBufferOutputNode(stereo[i]->outputNode(3), 0);
        combineNode[i]->setBufferOutputNode(stereo[i]->outputNode(0), 1);
        combineNode[i]->setInvertRenderingOrder(true);
        _combineMultipleScene[i] = combineNode[i];
    }

    if(param.useFxaa)
    {
        pipeline::SimpleFilter* antialiasNode[2];

        for(int i=0 ; i<2 ; ++i)
        {
            antialiasNode[i] = &_pipeline->createNode<pipeline::SimpleFilter>();
            antialiasNode[i]->setShader(ShaderPool::instance().get("fxaa"));
            antialiasNode[i]->setBufferOutputNode(combineNode[i], 0);
            hmdNode.setBufferOutputNode(antialiasNode[i], i);
        }
    }
    else
    {
        hmdNode.setBufferOutputNode(combineNode[0], 0);
        hmdNode.setBufferOutputNode(combineNode[1], 1);
    }

    _pipeline->setOutputNode(hmdNode);
}

/* Helper */

Pipeline::OutBuffersNode* FullPipeline::createSubDeferredPipeline(uivec2 res, const Parameter& param, int chanel)
{
    if(!_pipeline)
        return nullptr;

    Pipeline::DeferredRendererEntity& rendererEntity = _pipeline->genDeferredRendererEntity(res, param.usePointLight, param.usePostSSReflexion);
    rendererEntity.envLightRenderer().setEnableSSReflexion(param.useSSReflexion);
    rendererEntity.envLightRenderer().setEnableGI(true);

    pipeline::DeferredRendererNode& rendererNode = _pipeline->createNode<pipeline::DeferredRendererNode>();
    if(chanel > 0)
        rendererNode.setAuxiliar(true);

    pipeline::SimpleSceneMeshCullingNode& meshCuller = _pipeline->createNode<pipeline::SimpleSceneMeshCullingNode>();

    pipeline::SimpleSceneLightCullingNode* lightCuller = nullptr;
    pipeline::DirLightCullingNode<SimpleScene>* dirLightCuller = nullptr;
    pipeline::DirLightShadowNode* shadowRenderer = nullptr;

    if(param.usePointLight) lightCuller = &_pipeline->createNode<pipeline::SimpleSceneLightCullingNode>();
    if(param.useShadow) dirLightCuller = &_pipeline->createNode<pipeline::DirLightCullingNode<SimpleScene>>();
    if(param.useShadow) shadowRenderer = &_pipeline->createNode<pipeline::DirLightShadowNode>();

    rendererNode.setRendererEntity(rendererEntity);

    if(param.useShadow)
    {
        shadowRenderer->setDepthMapResolution(param.shadowResolution);
        shadowRenderer->setShadowLightRange(param.shadowCascad);
        shadowRenderer->addMeshInstanceCollector(*dirLightCuller);
        dirLightCuller->setDepthMapResolution(param.shadowResolution);
        dirLightCuller->setShadowLightRange(param.shadowCascad);
        rendererNode.setDirLightShadow(*shadowRenderer, 0);
    }

    rendererNode.addMeshInstanceCollector(meshCuller);
    if(param.usePointLight) rendererNode.addLightInstanceCollector(*lightCuller);

    _deferredEntities.insert(&rendererEntity);
    _deferredRendererNodes[0][chanel].push_back(&rendererNode);
    _meshCullingNodes[chanel].push_back(&meshCuller);

    if(lightCuller) _lightCullingNodes[chanel].push_back(lightCuller);
    if(dirLightCuller) _dirLightCullingNodes[chanel].push_back(dirLightCuller);
    if(shadowRenderer) _shadowMapNodes[chanel].push_back(shadowRenderer);

    return &rendererNode;
}

std::pair<Pipeline::OutBuffersNode*,Pipeline::OutBuffersNode*>
    FullPipeline::createSubStereoDeferredPipeline(uivec2 res, const Parameter& param, int chanel)
{
    if(!_pipeline)
        return {nullptr,nullptr};

    static int uniquId=1;
    Pipeline::DeferredRendererEntity& rendererEntity1 = _pipeline->genDeferredRendererEntity(res, param.usePointLight, param.usePostSSReflexion, uniquId++);
    Pipeline::DeferredRendererEntity& rendererEntity2 = _pipeline->genDeferredRendererEntity(res, param.usePointLight, param.usePostSSReflexion, uniquId++);
    rendererEntity1.envLightRenderer().setEnableSSReflexion(param.useSSReflexion);
    rendererEntity2.envLightRenderer().setEnableSSReflexion(param.useSSReflexion);
    rendererEntity1.envLightRenderer().setEnableGI(true);
    rendererEntity2.envLightRenderer().setEnableGI(true);

    pipeline::DeferredRendererNode& rendererNode1 = _pipeline->createNode<pipeline::DeferredRendererNode>();
    pipeline::DeferredRendererNode& rendererNode2 = _pipeline->createNode<pipeline::DeferredRendererNode>();
    if(chanel > 0)
    {
        rendererNode1.setAuxiliar(true);
        rendererNode2.setAuxiliar(true);
    }

    pipeline::SimpleSceneMeshCullingNode& meshCuller = _pipeline->createNode<pipeline::SimpleSceneMeshCullingNode>();

    pipeline::SimpleSceneLightCullingNode* lightCuller = nullptr;
    pipeline::DirLightCullingNode<SimpleScene>* dirLightCuller = nullptr;
    pipeline::DirLightShadowNode* shadowRenderer = nullptr;

    if(param.usePointLight) lightCuller = &_pipeline->createNode<pipeline::SimpleSceneLightCullingNode>();
    if(param.useShadow) dirLightCuller = &_pipeline->createNode<pipeline::DirLightCullingNode<SimpleScene>>();
    if(param.useShadow) shadowRenderer = &_pipeline->createNode<pipeline::DirLightShadowNode>();

    rendererNode1.setRendererEntity(rendererEntity1);
    rendererNode2.setRendererEntity(rendererEntity2);

    if(param.useShadow)
    {
        shadowRenderer->setDepthMapResolution(param.shadowResolution);
        shadowRenderer->setShadowLightRange(param.shadowCascad);
        shadowRenderer->addMeshInstanceCollector(*dirLightCuller);
        dirLightCuller->setDepthMapResolution(param.shadowResolution);
        dirLightCuller->setShadowLightRange(param.shadowCascad);
        rendererNode1.setDirLightShadow(*shadowRenderer, 0);
        rendererNode2.setDirLightShadow(*shadowRenderer, 0);
    }

    rendererNode1.addMeshInstanceCollector(meshCuller);
    rendererNode2.addMeshInstanceCollector(meshCuller);
    if(param.usePointLight)
    {
        rendererNode1.addLightInstanceCollector(*lightCuller);
        rendererNode2.addLightInstanceCollector(*lightCuller);
    }

    _deferredEntities.insert(&rendererEntity1);
    _deferredEntities.insert(&rendererEntity2);
    _deferredRendererNodes[0][chanel].push_back(&rendererNode1);
    _deferredRendererNodes[1][chanel].push_back(&rendererNode2);
    _meshCullingNodes[chanel].push_back(&meshCuller);

    if(lightCuller) _lightCullingNodes[chanel].push_back(lightCuller);
    if(dirLightCuller) _dirLightCullingNodes[chanel].push_back(dirLightCuller);
    if(shadowRenderer) _shadowMapNodes[chanel].push_back(shadowRenderer);

    return {&rendererNode1,&rendererNode2};
}

void FullPipeline::setNull()
{
    _deferredEntities.clear();
    _combineMultipleScene[0] = nullptr;
    _combineMultipleScene[1] = nullptr;

    for(int i=0 ; i<NB_CHANEL ; ++i)
    {
        _deferredRendererNodes[0][i].clear();
        _deferredRendererNodes[1][i].clear();
        _meshCullingNodes[i].clear();
        _lightCullingNodes[i].clear();
        _dirLightCullingNodes[i].clear();
        _shadowMapNodes[i].clear();
    }
}

}
}
#include "MemoryLoggerOff.h"
