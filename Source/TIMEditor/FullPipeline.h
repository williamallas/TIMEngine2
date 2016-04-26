#ifndef FULLPIPELINE_H
#define FULLPIPELINE_H

#include "interface/Pipeline.h"
#include "interface/pipeline/pipeline.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    struct FullPipeline : boost::noncopyable
    {
        ~FullPipeline();

        struct Parameter
        {
            bool useShadow = true;
            vector<float> shadowCascad = {30,100,300,1000};
            uint shadowResolution = 2048;

            bool usePointLight = true;
            bool useSSReflexion = false;
        };

        interface::Pipeline* pipeline = nullptr;
        interface::Pipeline::DeferredRendererEntity* rendererEntity = nullptr;

        interface::pipeline::OnScreenRenderer* onScreen = nullptr;

        interface::pipeline::DeferredRendererNode* rendererNode = nullptr;
        interface::pipeline::SimpleSceneMeshCullingNode* meshCuller = nullptr;
        interface::pipeline::SimpleSceneLightCullingNode* lightCuller = nullptr;

        interface::pipeline::DirLightCullingNode<interface::SimpleScene>* dirLightCuller = nullptr;
        interface::pipeline::DirLightShadowNode* shadowRenderer = nullptr;

        void create(uivec2, const Parameter&);
        void setScene(interface::Scene&, interface::View&);
        void setDirLightView(interface::View&);

    private:
        void setNull();
    };
}
#include "MemoryLoggerOff.h"

#endif // FULLPIPELINE_H
