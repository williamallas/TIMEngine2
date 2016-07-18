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
        vec3 offset;
    };

    MultipleSceneHelper(const interface::FullPipeline::Parameter&, interface::FullPipeline&);
    ~MultipleSceneHelper();

    void setResolution(uivec2 res) { _resolution = res; }

    void setCurScene(interface::Scene&);
    void setView(interface::View&);
    void setStereoView(interface::View&, interface::View&);
    void registerDirLightView(interface::Scene*, interface::View*);
    void addEdge(Edge);

    bool update(interface::Scene*&, vec3* offset=nullptr);
    void rebuild(interface::Scene&);

private:
    struct InternalEdge
    {
        Edge edge;

        Plan portalPlan;
        Box portalBox;
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

};

#endif // MULTIPLESCENEHELPER_H
