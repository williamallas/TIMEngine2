#ifndef MULTIPLESCENEHELPER_H
#define MULTIPLESCENEHELPER_H

#include "interface/FullPipeline.h"
#include "interface/MeshInstance.h"
using namespace tim;

class MultipleSceneHelper
{
public:
    struct Edge
    {
        interface::Geometry portalGeom;
        interface::MeshInstance *portal;
        interface::Scene *sceneFrom, *sceneTo;
        interface::MeshInstance* destPortal;
    };

    MultipleSceneHelper(const interface::FullPipeline::Parameter&, interface::FullPipeline&);
    ~MultipleSceneHelper();

    interface::FullPipeline& pipeline() { return _pipeline; }
    const interface::FullPipeline& pipeline() const { return _pipeline; }

    void setResolution(uivec2 res) { _resolution = res; }

    void setCurScene(interface::Scene&);
    void setView(interface::View&);
    void setStereoView(interface::View&, interface::View&);
    void registerDirLightView(interface::Scene*, interface::View*);
    void addEdge(Edge);
    void addEdge(interface::Scene *sceneFrom, interface::Scene *sceneTo,
                 interface::MeshInstance*, interface::Geometry, interface::MeshInstance* dest = nullptr);

    bool update(interface::Scene*&);
    void rebuild(interface::Scene&);

    void setEnableEdge(bool b, interface::Scene&, interface::MeshInstance*);

private:
    struct InternalEdge
    {
        Edge edge;

        Plan portalPlan;
        Box portalBox;
        bool enabled = true;
    };

    uivec2 _resolution;
    interface::FullPipeline::Parameter _param;
    interface::FullPipeline& _pipeline;

    int _nbExtraPipeline = 0;
    int _curNbEdge = 0;

    renderer::Shader* _combineSceneShader;
    int _nbSceneUniformId = -1;

    interface::Scene* _currentScene = nullptr;
    interface::View* _curCamera = nullptr;
    interface::View* _curStereoCamera[2] = {nullptr, nullptr};
    vec3 _lastCameraPos;
    boost::container::map<interface::Scene*, vector<InternalEdge>> _graph;
    vector<interface::View*> _extraCameras;
    vector<interface::View*> _extraStereoCameras[2];

    boost::container::map<interface::Scene*, interface::View*> _dirLightView;

    void freeCamera();
    void constructEdge(const InternalEdge&);
    void optimizeExtraSceneRendering(InternalEdge&, int i);

    bool getScreenBoundingBox(const Box&, const mat4& boxMat, const mat4& projView, vec2& minV, vec2& maxV);

};

#endif // MULTIPLESCENEHELPER_H
