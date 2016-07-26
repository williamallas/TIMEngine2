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
        static const int NB_CHANEL = 8;
        ~FullPipeline();

        struct Parameter
        {
            bool useShadow = false;
            vector<float> shadowCascad = {30,100,300,1000};
            uint shadowResolution = 2048;

            bool usePointLight = false;
            bool usePostSSReflexion = false;
            bool useSSReflexion = false;
            bool useFxaa = true;
        };

        Pipeline* pipeline() const { return _pipeline; }
        bool isStereo() const { return _stereoscopy; }

        Pipeline::InOutBufferNode* combineNode(int index=0) const;
        const vector<pipeline::DeferredRendererNode*>& deferredRendererNode(int chanel, int eye=0) const;


        const std::set<Pipeline::DeferredRendererEntity*>& rendererEntities() const { return _deferredEntities; }

        void create(uivec2, const Parameter&);
        void createTwoScene(uivec2, const Parameter&, const Parameter&);
        void createExtensible(uivec2, const Parameter&);

        void setScene(Scene&, View&, int sceneId);

        /* for stereoscopy */
        void createStereo(Pipeline::TerminalNode&, uivec2, const Parameter&);
        void createStereoExtensible(Pipeline::TerminalNode& hmdNode, uivec2, const Parameter&);
        void setStereoView(View&, View& eye1, View& eye2, int sceneId);

        /* for both */
        void extendPipeline(uivec2, const Parameter&, int index);
        void setDirLightView(View&, int sceneId);
        void setScene(Scene&, int sceneId);

    private:
        void setNull();
        Pipeline::OutBuffersNode* createSubDeferredPipeline(uivec2, const Parameter&, int);
        std::pair<Pipeline::OutBuffersNode*,Pipeline::OutBuffersNode*> createSubStereoDeferredPipeline(uivec2, const Parameter&, int);

        Pipeline* _pipeline = nullptr;
        bool _stereoscopy = false;

        std::set<Pipeline::DeferredRendererEntity*> _deferredEntities;
        vector<pipeline::DeferredRendererNode*> _deferredRendererNodes[2][NB_CHANEL];

        vector<pipeline::SimpleSceneMeshCullingNode*> _meshCullingNodes[NB_CHANEL];
        vector<pipeline::SimpleSceneLightCullingNode*> _lightCullingNodes[NB_CHANEL];

        vector< pipeline::DirLightCullingNode<SimpleScene>* > _dirLightCullingNodes[NB_CHANEL];
        vector< Pipeline::DepthMapRendererNode* > _shadowMapNodes[NB_CHANEL];

        Pipeline::InOutBufferNode* _combineMultipleScene[2] = {nullptr, nullptr};
    };
}
}
#include "MemoryLoggerOff.h"

#endif // FULLPIPELINE_H
