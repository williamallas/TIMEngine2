#ifndef FULLPIPELINE_H
#define FULLPIPELINE_H

#include "interface/Pipeline.h"
#include "interface/pipeline/pipeline.h"
#include "interface/ShaderPool.h"

#include "MemoryLoggerOn.h"
#undef interface
namespace tim
{
namespace interface
{
    struct FullPipeline : boost::noncopyable
    {
        ~FullPipeline();

        struct Parameter
        {
            bool useShadow = false;
            vector<float> shadowCascad = {30,100,300,1000};
            uint shadowResolution = 2048;

            bool usePointLight = false;
            bool useSSReflexion = false;
        };

        Pipeline* pipeline = nullptr;
        Pipeline::DeferredRendererEntity* rendererEntity = nullptr;

        pipeline::SimpleSceneMeshCullingNode* meshCuller = nullptr;
        pipeline::SimpleSceneLightCullingNode* lightCuller = nullptr;

        pipeline::DirLightCullingNode<SimpleScene>* dirLightCuller = nullptr;
        pipeline::DirLightShadowNode* shadowRenderer = nullptr;

		/* If not hmd */
		pipeline::OnScreenRenderer* onScreen = nullptr;
		pipeline::DeferredRendererNode* rendererNode = nullptr;

		/* Extra node for HMD */
        Pipeline::TerminalNode* hmdNode = nullptr;
		pipeline::DeferredRendererNode* hmdEyeRendererNode[2] = { nullptr, nullptr };
		Pipeline::DeferredRendererEntity* secondRendererEntity = nullptr;
        pipeline::SimpleFilter* antialiasingFilter[2] = {nullptr, nullptr};

        void create(uivec2, const Parameter&);
		
		template<class T>
		void createForHmd(uivec2, const Parameter&);

        void setScene(Scene&, View&);
        void setDirLightView(View&);

    private:
        void setNull();
    };

	template<class T>
	void FullPipeline::createForHmd(uivec2 res, const Parameter& param)
	{
		delete pipeline;
		setNull();

		pipeline = new interface::Pipeline;
		rendererEntity = &pipeline->genDeferredRendererEntity(res, param.usePointLight, param.useSSReflexion, 0);
		secondRendererEntity = &pipeline->genDeferredRendererEntity(res, param.usePointLight, param.useSSReflexion, 1);
		onScreen = nullptr;

		hmdEyeRendererNode[0] = &pipeline->createNode<pipeline::DeferredRendererNode>();
		hmdEyeRendererNode[1] = &pipeline->createNode<pipeline::DeferredRendererNode>();

		meshCuller = &pipeline->createNode<pipeline::SimpleSceneMeshCullingNode>();
		if (param.usePointLight) lightCuller = &pipeline->createNode<pipeline::SimpleSceneLightCullingNode>();
		if (param.useShadow) dirLightCuller = &pipeline->createNode<pipeline::DirLightCullingNode<SimpleScene>>();
		if (param.useShadow) shadowRenderer = &pipeline->createNode<pipeline::DirLightShadowNode>();

		hmdEyeRendererNode[0]->setRendererEntity(*rendererEntity);
		hmdEyeRendererNode[1]->setRendererEntity(*secondRendererEntity);

		if (param.useShadow)
		{
			shadowRenderer->setDepthMapResolution(param.shadowResolution);
			shadowRenderer->setShadowLightRange(param.shadowCascad);
			shadowRenderer->addMeshInstanceCollector(*dirLightCuller);
			dirLightCuller->setDepthMapResolution(param.shadowResolution);
			dirLightCuller->setShadowLightRange(param.shadowCascad);

			hmdEyeRendererNode[0]->setDirLightShadow(*shadowRenderer, 0);
			hmdEyeRendererNode[1]->setDirLightShadow(*shadowRenderer, 0);
		}

		hmdEyeRendererNode[0]->addMeshInstanceCollector(*meshCuller);
		hmdEyeRendererNode[1]->addMeshInstanceCollector(*meshCuller);

		if (param.usePointLight)
		{
			hmdEyeRendererNode[0]->addLightInstanceCollector(*lightCuller);
			hmdEyeRendererNode[1]->addLightInstanceCollector(*lightCuller);
		}

        antialiasingFilter[0] = &pipeline->createNode<pipeline::SimpleFilter>();
        antialiasingFilter[1] = &pipeline->createNode<pipeline::SimpleFilter>();
        antialiasingFilter[0]->setShader(ShaderPool::instance().get("fxaa"));
        antialiasingFilter[1]->setShader(ShaderPool::instance().get("fxaa"));

        antialiasingFilter[0]->setBufferOutputNode(hmdEyeRendererNode[0]->outputNode(0),0);
        antialiasingFilter[1]->setBufferOutputNode(hmdEyeRendererNode[1]->outputNode(0),0);

		hmdNode = &pipeline->createNode<T>();
        hmdNode->setBufferOutputNode(antialiasingFilter[0], 0);
        hmdNode->setBufferOutputNode(antialiasingFilter[1], 1);
		pipeline->setOutputNode(*hmdNode);
	}
}
}
#include "MemoryLoggerOff.h"

#endif // FULLPIPELINE_H
