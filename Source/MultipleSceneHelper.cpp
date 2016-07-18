#include "MultipleSceneHelper.h"

MultipleSceneHelper::MultipleSceneHelper(const interface::FullPipeline::Parameter& param, interface::FullPipeline& pipe)
    : _param(param), _pipeline(pipe)
{
    _combineSceneShader = interface::ShaderPool::instance().get("combineScene");
    _nbSceneUniformId = _combineSceneShader->uniformLocation("nbScene");
}

MultipleSceneHelper::~MultipleSceneHelper()
{
    freeCamera();
}

void MultipleSceneHelper::setView(interface::View& view)
{
    _curCamera = &view;
}

void MultipleSceneHelper::setCurScene(interface::Scene& scene)
{
    if(_currentScene == &scene)
        return;

    rebuild(scene);
}

void MultipleSceneHelper::registerDirLightView(interface::Scene* scene, interface::View* dirView)
{
    _dirLightView[scene] = dirView;
}

void MultipleSceneHelper::addEdge(Edge edge)
{
    if(edge.sceneFrom == nullptr || edge.sceneTo == nullptr || edge.portal == nullptr)
        return;

    if(edge.portalGeom.meshData() == nullptr)
        return;

    InternalEdge internEdge;
    internEdge.edge = edge;
    internEdge.portalBox = Box::computeBox(reinterpret_cast<const real*>(edge.portalGeom.meshData()->vData),
                                           edge.portalGeom.meshData()->nbVertex,
                                           sizeof(renderer::MeshData::DataType)/sizeof(real));

    internEdge.portalPlan = internEdge.portalBox.extractOptimalPlan();


    vector<InternalEdge>& add = _graph[edge.sceneFrom];
    add.push_back(internEdge);

    if(edge.sceneFrom == _currentScene)
    {
        constructEdge(internEdge);
    }
    else
    {
        interface::Mesh m=edge.portal->mesh();
        if(m.nbElements() > 0) m.element(0).setEnable(true);
        edge.portal->setMesh(m);
    }
}

void MultipleSceneHelper::rebuild(interface::Scene& scene)
{
    if(_currentScene)
    {
        vector<InternalEdge>& edgeToFlush = _graph[_currentScene];
        for(InternalEdge& e : edgeToFlush)
        {
            interface::Mesh m=e.edge.portal->mesh();
            if(m.nbElements() > 0) m.element(0).setEnable(false);
            e.edge.portal->setMesh(m);
        }
    }

    vector<InternalEdge>& add = _graph[&scene];

    _currentScene = &scene;
    _nbExtraPipeline = 0;
    _curNbEdge = 0;

    freeCamera();

    for(size_t i=0 ; i<add.size() ; ++i)
    {
        constructEdge(add[i]);
    }

    for(size_t i=add.size() ; i<size_t(_nbExtraPipeline) ; ++i)
    {
        _pipeline.combineNode(0)->setEnableInput(i+1, false);
        if(_pipeline.isStereo())
            _pipeline.combineNode(1)->setEnableInput(i+1, false);
    }
}

bool MultipleSceneHelper::update(interface::Scene*& sceneCrossed, vec3* offsetScene)
{
    if(!_curCamera)
        return false;

    bool res=false;

    {
    vector<InternalEdge>& curEdges = _graph[_currentScene];

    for(size_t i=0 ; i<curEdges.size() ; ++i)
    {
        Plan transformedPlan = curEdges[i].portalPlan.transformed(curEdges[i].edge.portal->matrix());
        if(transformedPlan.distance(_curCamera->camera.pos) > 0)
            transformedPlan = Plan(transformedPlan.plan() * -1);

        float d1 = transformedPlan.distance(_curCamera->camera.pos);
        float d2 = transformedPlan.distance(_lastCameraPos);

        if(d2 > 0 && d2-d1 < 0.2) // cross the plan
        {
            vec3 pInter = (_lastCameraPos*(-d1) + _curCamera->camera.pos*d2) / (-d1+d2);

            if(curEdges[i].portalBox.inside(curEdges[i].edge.portal->matrix().inverted()*pInter))
            {
                sceneCrossed = curEdges[i].edge.sceneTo;
                if(offsetScene) *offsetScene = curEdges[i].edge.offset;
                res = true;

                rebuild(*sceneCrossed);
                break;
            }
        }
    }
    }

    vector<InternalEdge>& curEdges = _graph[_currentScene];

    for(size_t i=0 ; i<_extraCameras.size() ; ++i)
    {
        if(!_extraCameras[i])
            continue;

        *_extraCameras[i] = *_curCamera;
        _extraCameras[i]->offset(curEdges[i].edge.offset);
        if(_pipeline.isStereo())
        {
            *_extraStereoCameras[0][i] = *_curStereoCamera[0];
            _extraStereoCameras[0][i]->offset(curEdges[i].edge.offset);

            *_extraStereoCameras[1][i] = *_curStereoCamera[1];
            _extraStereoCameras[1][i]->offset(curEdges[i].edge.offset);
        }
    }

    for(size_t i=0 ; i<curEdges.size() ; ++i)
    {
        Plan transformedPlan = curEdges[i].portalPlan.transformed(curEdges[i].edge.portal->matrix());
        if(transformedPlan.distance(_curCamera->camera.pos) > 0)
            transformedPlan = Plan(transformedPlan.plan() * -1);

        interface::Mesh m = curEdges[i].edge.portal->mesh();
        if(m.nbElements() > 0)
        {
            m.element(0).setColor(transformedPlan.plan());
            m.element(0).setSpecular(0.2 * i);
            curEdges[i].edge.portal->setMesh(m);
        }

        for(auto ptr : _pipeline.deferredRendererNode(i+1, 0))
        {
            ptr->setClipPlan(transformedPlan.plan(), 0);
            ptr->setUseClipPlan(true, 0);
        }

        if(_pipeline.isStereo())
        {
            for(auto ptr : _pipeline.deferredRendererNode(i+1, 1))
            {
                ptr->setClipPlan(transformedPlan.plan(), 0);
                ptr->setUseClipPlan(true, 0);
            }
        }
    }

    _lastCameraPos = _curCamera->camera.pos;

    return res;
}

void MultipleSceneHelper::constructEdge(const InternalEdge& edge)
{
    for(int i=_nbExtraPipeline ; i<_curNbEdge+1 ; ++i)
    {
        _pipeline.extendPipeline(_resolution, _param, i+1);
        _extraCameras.push_back(nullptr);
        _extraStereoCameras[0].push_back(nullptr);
        _extraStereoCameras[1].push_back(nullptr);
        _nbExtraPipeline = i+1;
    }

    _extraCameras[_curNbEdge] = new interface::View;

    if(_pipeline.isStereo())
    {
        _extraStereoCameras[0][_curNbEdge] = new interface::View;
        _extraStereoCameras[1][_curNbEdge] = new interface::View;
        _pipeline.setScene(*edge.edge.sceneTo, _curNbEdge+1);
        interface::View* tmp[2] = {_extraStereoCameras[0][_curNbEdge], _extraStereoCameras[1][_curNbEdge]};
        _pipeline.setStereoView(*_extraCameras[_curNbEdge], tmp, _curNbEdge+1);
    }
    else
    {
        _pipeline.setScene(*edge.edge.sceneTo, *_extraCameras[_curNbEdge], _curNbEdge+1);
    }

    interface::View* dirLightView = _dirLightView[edge.edge.sceneTo];
    if(dirLightView)
        _pipeline.setDirLightView(*dirLightView, _curNbEdge+1);

    _curNbEdge++;

    _pipeline.combineNode(0)->setEnableInput(_curNbEdge, true);

     if(_pipeline.isStereo())
     {
         _pipeline.combineNode(1)->setEnableInput(_curNbEdge, true);
     }

     interface::Mesh m=edge.edge.portal->mesh();
     if(m.nbElements() > 0) m.element(0).setEnable(true);
     edge.edge.portal->setMesh(m);
}

void MultipleSceneHelper::freeCamera()
{
    for(size_t i=0 ; i<_extraCameras.size() ; ++i)
    {
        delete _extraCameras[i];
        delete _extraStereoCameras[0][i];
        delete _extraStereoCameras[1][i];
    }

    _extraCameras.clear();
    _extraStereoCameras[0].clear();
    _extraStereoCameras[1].clear();
}


