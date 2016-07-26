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

void MultipleSceneHelper::setEnableEdge(bool b, interface::Scene& sceneFrom, interface::MeshInstance* inst)
{
    vector<InternalEdge>& candidat = _graph[&sceneFrom];
    for(InternalEdge& e : candidat)
    {
        if(e.edge.portal == inst)
            e.enabled = b;
    }
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

void MultipleSceneHelper::setStereoView(interface::View& eye1, interface::View& eye2)
{
    _curStereoCamera[0] = &eye1;
    _curStereoCamera[1] = &eye2;
}

void MultipleSceneHelper::registerDirLightView(interface::Scene* scene, interface::View* dirView)
{
    _dirLightView[scene] = dirView;
}

void MultipleSceneHelper::addEdge(interface::Scene *sceneFrom, interface::Scene *sceneTo,
             interface::MeshInstance* inst, interface::Geometry geom, interface::MeshInstance* dest)
{
    Edge e;
    e.destPortal = dest;
    e.portal = inst;
    e.portalGeom = geom;
    e.sceneFrom = sceneFrom;
    e.sceneTo = sceneTo;
    addEdge(e);
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
        if(m.nbElements() > 0) m.element(0).setEnable(1);
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
            if(m.nbElements() > 0)
                m.element(0).setEnable(0);
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
        _pipeline.combineNode(0)->setEnableInput(i+2, false);
        if(_pipeline.isStereo())
            _pipeline.combineNode(1)->setEnableInput(i+2, false);
    }
}

bool MultipleSceneHelper::update(interface::Scene*& sceneCrossed)
{
    if(!_curCamera)
        return false;

    bool res=false;

    {
    vector<InternalEdge>& curEdges = _graph[_currentScene];

    for(size_t i=0 ; i<curEdges.size() ; ++i)
    {
        if(!curEdges[i].enabled)
            continue;

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
                res = true;

                rebuild(*sceneCrossed);

                vec3 offset = curEdges[i].edge.destPortal->matrix().translation() - curEdges[i].edge.portal->matrix().translation();
                _curCamera->offset(offset);

                if(_pipeline.isStereo())
                {
                    _curStereoCamera[0]->offset(offset);
                    _curStereoCamera[1]->offset(offset);
                }

                interface::View* v = _dirLightView[sceneCrossed];
                if(v)
                {
                    _pipeline.setDirLightView(*v, 0);
                    v->dirLightView.camPos = _curCamera->camera.pos;
                }

                _pipeline.setScene(*sceneCrossed, 0);
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

        interface::View* view = _dirLightView[curEdges[i].edge.sceneTo];
        if(view)
            _extraCameras[i]->dirLightView = view->dirLightView;

        vec3 offset = curEdges[i].edge.destPortal->matrix().translation() - curEdges[i].edge.portal->matrix().translation();
        _extraCameras[i]->offset(offset);
        _extraCameras[i]->dirLightView.camPos =_extraCameras[i]->camera.pos;

        if(_pipeline.isStereo())
        {
            *_extraStereoCameras[0][i] = *_curStereoCamera[0];
            _extraStereoCameras[0][i]->offset(offset);

            *_extraStereoCameras[1][i] = *_curStereoCamera[1];
            _extraStereoCameras[1][i]->offset(offset);

            if(!curEdges[i].enabled)
            {
                _pipeline.combineNode(0)->setEnableInput(i+2, false);
                if(_pipeline.isStereo())
                    _pipeline.combineNode(1)->setEnableInput(i+2, false);

                continue;
            }

            optimizeExtraSceneRendering(curEdges[i], i);
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
            m.element(0).setEnable(curEdges[i].enabled ? 1:0);
            m.element(0).setMaterial(transformedPlan.plan());
            m.element(0).setTextureScale(0.2 * i);
            m.element(0).drawState().setCullFace(false);
            m.element(0).drawState().setShader(interface::ShaderPool::instance().get("portalShader"));
            curEdges[i].edge.portal->setMesh(m);
        }

        vec3 offset = curEdges[i].edge.destPortal->matrix().translation() - curEdges[i].edge.portal->matrix().translation();
        vec4 planNext = transformedPlan.transformed(mat4::Translation(offset)).plan();
        for(auto ptr : _pipeline.deferredRendererNode(i+1, 0))
        {
            ptr->setClipPlan(planNext, 0);
            ptr->setUseClipPlan(true, 0);
        }

        if(_pipeline.isStereo())
        {
            for(auto ptr : _pipeline.deferredRendererNode(i+1, 1))
            {
                ptr->setClipPlan(planNext, 0);
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
        _pipeline.setStereoView(*_extraCameras[_curNbEdge], *tmp[0], *tmp[1], _curNbEdge+1);
    }
    else
    {
        _pipeline.setScene(*edge.edge.sceneTo, *_extraCameras[_curNbEdge], _curNbEdge+1);
    }

    _pipeline.setDirLightView(*_extraCameras[_curNbEdge], _curNbEdge+1);

    _curNbEdge++;

    _pipeline.combineNode(0)->setEnableInput(_curNbEdge+1, true);

    if(_pipeline.isStereo())
    {
        _pipeline.combineNode(1)->setEnableInput(_curNbEdge+1, true);
    }

    interface::Mesh m=edge.edge.portal->mesh();
    if(m.nbElements() > 0)
    {
        m.element(0).setEnable(1);
        m.element(0).setCastShadow(false);
    }
    edge.edge.portal->setMesh(m);
}

void MultipleSceneHelper::optimizeExtraSceneRendering(InternalEdge& curEdge, int i)
{
    Box portalBox = curEdge.portalBox;

    mat4 portalMatrix = curEdge.edge.portal->matrix();
    mat4 invPortalMatrix = portalMatrix.inverted();

    Frustum frust;
    Camera cam = _curCamera->camera;
    cam.pos = invPortalMatrix * cam.pos;
    cam.dir = invPortalMatrix * cam.dir;
    cam.up = invPortalMatrix.down<1>() * cam.up;
    frust.buildCameraFrustum(cam);
    if(!frust.collide(portalBox))
    {
        _pipeline.combineNode(0)->setEnableInput(i+2, false);
        if(_pipeline.isStereo())
            _pipeline.combineNode(1)->setEnableInput(i+2, false);
    }
    else
    {
        _pipeline.combineNode(0)->setEnableInput(i+2, true);
        if(_pipeline.isStereo())
            _pipeline.combineNode(1)->setEnableInput(i+2, true);
    }

    //frutum culling optimization
    vec3 offset = curEdge.edge.destPortal->matrix().translation() - curEdge.edge.portal->matrix().translation();
    cam = _curCamera->camera;
    cam.dir = portalMatrix * portalBox.center();
    cam.pos += offset;
    cam.dir += offset;
    vec3 dirView = (cam.dir-cam.pos).normalized();

    float maxAngle = 0;
    float near = 99999;

    for(int x=0 ; x<2 ; ++x)for(int y=0 ; y<2 ; ++y)for(int z=0 ; z<2 ; ++z)
    {
        vec3 p = {portalBox.box()[0][x], portalBox.box()[1][y], portalBox.box()[2][z]};
        p = (portalMatrix * p);

        p += offset;
        float angle = acosf(dirView.dot((p-cam.pos).normalized()));
        maxAngle = std::max(maxAngle, angle);
        near = std::min(near, (p-cam.pos).length());
    }

    near = std::max(0.1f, near);

    if(_curCamera->camera.fov > toDeg(maxAngle)*2)
    {
        cam.clipDist.x() = near;
        cam.fov = toDeg(maxAngle)*2;
        _extraCameras[i]->camera = cam;
    }

    if(_pipeline.isStereo())
    {
        vec2 minV, maxV;
        mat4 projView = _curStereoCamera[0]->camera.raw_proj * _curStereoCamera[0]->camera.raw_view;
        bool useScissor = getScreenBoundingBox(portalBox, portalMatrix, projView, minV, maxV);

        for(auto ptr : _pipeline.deferredRendererNode(i+1, 0))
            ptr->setScissorTest(useScissor, minV, maxV-minV);

        projView = _curStereoCamera[1]->camera.raw_proj * _curStereoCamera[1]->camera.raw_view;
        useScissor = getScreenBoundingBox(portalBox, portalMatrix, projView, minV, maxV);

        for(auto ptr : _pipeline.deferredRendererNode(i+1, 1))
            ptr->setScissorTest(useScissor, minV, maxV-minV);
    }
}

bool MultipleSceneHelper::getScreenBoundingBox(const Box& box, const mat4& boxMat, const mat4& projView, vec2& minV, vec2& maxV)
{
    minV = {1,1};
    maxV = {-1,-1};

    bool everyThingIn = true;
    for(int x=0 ; x<2 ; ++x)for(int y=0 ; y<2 ; ++y)for(int z=0 ; z<2 ; ++z)
    {
        vec3 p = {box.box()[0][x], box.box()[1][y], box.box()[2][z]};
        p = (boxMat * p);
        vec4 projectedP = projView * vec4(p,1);

        if(projectedP.w() < 0)
            everyThingIn = false;

        projectedP /= fabs(projectedP.w());
        minV.x() = std::min(projectedP.x(), minV.x());
        minV.y() = std::min(projectedP.y(), minV.y());
        maxV.x() = std::max(projectedP.x(), maxV.x());
        maxV.y() = std::max(projectedP.y(), maxV.y());
    }

    if(everyThingIn)
    {
        minV.x() = std::max(-1.f,minV.x());
        minV.y() = std::max(-1.f,minV.y());
        maxV.x() = std::min( 1.f,maxV.x());
        maxV.y() = std::min( 1.f,maxV.y());

        minV = (minV * 0.5) + vec2(0.5,0.5);
        maxV = (maxV * 0.5) + vec2(0.5,0.5);

        if(minV.x() > maxV.x() || minV.y() > maxV.y())
        {
            minV = {0,0};
            maxV = {1,1};
            return false;
        }

        return true;
   }
    else
    {
        minV = {0,0};
        maxV = {1,1};
        return false;
    }
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


