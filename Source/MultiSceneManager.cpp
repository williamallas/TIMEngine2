#include "MultiSceneManager.h"
#include "resource/AssetManager.h"
#include "PortalGame/CollisionMask.h"
#include <boost/timer/timer.hpp>

using namespace interface;

#include "MemoryLoggerOn.h"

std::map<std::string,std::string> mapStringGeometry;
std::string mapGeometry(std::string str)
{
    auto it = mapStringGeometry.find(str);
    if(it == mapStringGeometry.end())
        return str;
    else return it->second;
}

MultiSceneManager::MultiSceneManager(std::string file, MultipleSceneHelper& multipleScene, int startScene)
{
    std::ifstream fs(file);
    if(!fs.is_open())
        return;

    struct EdgeInfo
    {
        XmlSceneLoader::ObjectLoaded xmlObj;
        std::string toObj;
        interface::Scene* scene;
    };

    mapStringGeometry["meshBank/portal5Frame.obj"] = "meshBank/portal5Frame_Pass.obj";
    mapStringGeometry["meshBank/portal5FrameR.obj"] = "meshBank/portal5FrameR_Pass.obj";

    interface::Geometry roomLimitGeom = resource::AssetManager<Geometry>::instance().load<false>("meshBank/roomPattern.obj", true).value();

    std::map<std::string, EdgeInfo> nameEdge;
    int index=0;
    while(fs)
    {
        std::string sceneFile;
        std::getline(fs, sceneFile);

        interface::Scene* scene = new interface::Scene;
        vector<XmlSceneLoader::ObjectLoaded> objInScene;

        bool b;
        {
            boost::timer::auto_cpu_timer t;std::cout << "Loading " << sceneFile << ":";
            b = XmlSceneLoader::loadScene(sceneFile, *scene, objInScene);
        }

        if(b)
        {
            sceneFile.resize(sceneFile.size()-4);
            _scenes.push_back({sceneFile, scene});
            _objects.push_back(objInScene);
            _dirLightView.push_back(new interface::View());

            if(index == startScene)
            {
                multipleScene.pipeline().setScene(*scene, 0);
                multipleScene.pipeline().setDirLightView(*_dirLightView.back(), 0);

                multipleScene.setCurScene(*scene);
            }

            if(scene->globalLight.dirLights.size() > 0)
                _dirLightView.back()->dirLightView.lightDir = scene->globalLight.dirLights[0].direction;

            multipleScene.registerDirLightView(scene, _dirLightView.back());

            vector<GeometryShape::MeshInstance> staticGeom, staticRoomGeom;
            for(XmlSceneLoader::ObjectLoaded& obj : objInScene)
            {
                if(obj.type == XmlSceneLoader::ObjectLoaded::MESH_INSTANCE)
                {
                    const std::string prefix = "portal";
                    if(obj.name.compare(0, prefix.size(), prefix) == 0)
                    {
                        vector<std::string> w = StringUtils(obj.name.substr(prefix.size(), obj.name.size()-prefix.size())).splitWord('_');
                        if(w.empty())
                            continue;

                        nameEdge[w[0]] = {obj, w.size()>1?w[1]:"", scene};

                        if(obj.isPhysic && obj.isStatic)
                        {
                            for(size_t i=1 ; i<obj.asset.size() ; ++i)
                            {
                                staticGeom.push_back({resource::AssetManager<Geometry>::instance().load<false>(mapGeometry(obj.asset[i].geometry), true).value(),
                                                      obj.meshInstance->matrix()});
                            }
                        }
                    }

                    else if(obj.collider.type == XmlSceneLoader::Collider::NONE && obj.name == "roomPattern")
                    {
                        staticRoomGeom.push_back({roomLimitGeom, mat4::constructTransformation(obj.rotation, obj.translation, vec3(1,1,1))});
                    }
                    else if(obj.isPhysic && obj.isStatic && obj.collider.type == XmlSceneLoader::Collider::NONE)
                    {
                        for(XmlMeshAssetLoader::MeshElementModel part : obj.asset)
                        {
                            if(obj.meshInstance)
                                staticGeom.push_back({resource::AssetManager<Geometry>::instance().load<false>(mapGeometry(part.geometry), true).value(),
                                                      obj.meshInstance->matrix()});
                        }
                    }
                }
            }

            btBvhTriangleMeshShape* shape = GeometryShape::genStaticGeometryShape(staticGeom);
            _staticGeom.push_back(shape);

            if(staticRoomGeom.empty())
                _staticRoomGeom.push_back(nullptr);
            else
            {
                btBvhTriangleMeshShape* roomShape = GeometryShape::genStaticGeometryShape(staticRoomGeom);
                _staticRoomGeom.push_back(roomShape);
            }

            ++index;
        }
    }

    for(auto e : nameEdge)
    {
        if(e.second.toObj.empty())
            continue;

        std::cout << "Edge:" << e.first << " -> " << e.second.toObj << std::endl;

        const EdgeInfo& toEdge = nameEdge[e.second.toObj];
        MeshInstance* inst = toEdge.xmlObj.meshInstance;

        if(!inst)
            continue;

        Geometry geom = resource::AssetManager<Geometry>::instance().load<false>(e.second.xmlObj.asset[0].geometry, true).value();
        multipleScene.addEdge(e.second.scene, toEdge.scene, e.second.xmlObj.meshInstance, geom, inst);
    }

}

MultiSceneManager::~MultiSceneManager()
{
    for(auto ptr : _scenes)
        delete ptr.second;

    for(auto ptr : _dirLightView)
        delete ptr;

    for(auto ptr : _staticGeom)
        delete ptr;

    for(auto ptr : _staticRoomGeom)
        delete ptr;

    for(auto ptr : _staticGeomObj)
        delete ptr;

    for(auto ptr : _staticRoomGeomObj)
        delete ptr;

    for(auto ptr : _sphereShapes)
        delete ptr.second;

    for(auto ptr : _boxShapes)
        delete ptr.second;
}

#include "MemoryLoggerOff.h"
void MultiSceneManager::instancePhysic(BulletEngine& bulletEngine)
{
    for(size_t i=0 ; i<_staticGeom.size() ; ++i)
    {
        _physObjects.push_back({});
        if(i > 0)
            bulletEngine.createWorld(i);

        BulletObject* obj = new BulletObject(mat4::IDENTITY(), _staticGeom[i]);
        bulletEngine.addObject(obj, i, CollisionTypes::COL_STATIC, STATIC_COLLISION);
        obj->body()->setRestitution(0.8);
        obj->body()->setFriction(0.75);

        _staticGeomObj.push_back(obj);

        if(_staticRoomGeom[i])
        {
            BulletObject* obj = new BulletObject(mat4::IDENTITY(), _staticRoomGeom[i]);
            bulletEngine.addObject(obj, i, CollisionTypes::COL_ROOM, ROOMPATTERN_COLLISION);
            obj->body()->setRestitution(0);
            obj->body()->setFriction(0);

            _staticRoomGeomObj.push_back(obj);
        }
        else
            _staticRoomGeomObj.push_back(nullptr);

        for(size_t j=0 ; j<_objects[i].size() ; ++j)
        {
            XmlSceneLoader::ObjectLoaded& obj = _objects[i][j];
            if(obj.isPhysic && !obj.isStatic)
            {
                BulletObject* pObj = nullptr;
                switch(obj.collider.type)
                {
                case XmlSceneLoader::Collider::NONE: break;

                case XmlSceneLoader::Collider::AUTO_BOX:
                {
                    if(obj.asset.empty()) break;

                    Box finalBox;
                    for(size_t k=0 ; k<std::min(1u,obj.asset.size()) ; ++k)
                    {
                        Geometry geom = resource::AssetManager<Geometry>::instance().load<false>(obj.asset[k].geometry, true).value();
                        Box b = Box::computeBox(reinterpret_cast<const real*>(geom.meshData()->vData),
                                                geom.meshData()->nbVertex,
                                                sizeof(renderer::MeshData::DataType)/sizeof(real));
                        b = b.extractOriginAlignedBox();
                        finalBox = finalBox.max(b);
                    }

                    btBoxShape* boxShape;
                    finalBox.setMin(finalBox.min() * obj.scale.x());
                    finalBox.setMax(finalBox.max() * obj.scale.x());
                    auto it = _boxShapes.find(finalBox.max());
                    if(it == _boxShapes.end())
                    {
                        boxShape = new btBoxShape(btVector3(finalBox.box()[0].y(), finalBox.box()[1].y(), finalBox.box()[2].y()));
                        _boxShapes[finalBox.max()] = boxShape;
                    }
                    else
                        boxShape = it->second;

                    pObj = new BulletObject(new SceneMotionState<MeshInstance>(*obj.meshInstance, obj.scale.x()), boxShape, obj.collider.mass);
                    break;
                }

                case XmlSceneLoader::Collider::AUTO_SPHERE:
                {
                    if(obj.asset.empty()) break;

                    Sphere finalSphere;
                    for(size_t k=0 ; k<std::min(1u,obj.asset.size()) ; ++k)
                    {
                        Geometry geom = resource::AssetManager<Geometry>::instance().load<false>(obj.asset[k].geometry, true).value();
                        Sphere s = Sphere::computeSphere(reinterpret_cast<const real*>(geom.meshData()->vData),
                                                      geom.meshData()->nbVertex,
                                                      sizeof(renderer::MeshData::DataType)/sizeof(real));
                        s = s.extractOriginAlignedSphere();
                        finalSphere = finalSphere.max(s);
                    }

                    finalSphere.setRadius(finalSphere.radius() * obj.scale.x());

                    btSphereShape* shape;
                    auto it = _sphereShapes.find(finalSphere.radius());
                    if(it == _sphereShapes.end())
                    {
                        shape = new btSphereShape(finalSphere.radius());
                        _sphereShapes[finalSphere.radius()] = shape;
                    }
                    else
                        shape = it->second;

                    pObj = new BulletObject(new SceneMotionState<MeshInstance>(*obj.meshInstance, obj.scale.x()), shape, obj.collider.mass);

                    break;
                }

                case XmlSceneLoader::Collider::CONVEX_HULL: break;
                }

                if(pObj)
                {
                    bulletEngine.addObject(pObj, i, CollisionTypes::COL_PHYS, PHYS_COLLISION);
                    pObj->body()->setRestitution(obj.collider.restitution);
                    pObj->body()->setFriction(obj.collider.friction);
                    pObj->body()->setRollingFriction(obj.collider.rollingFriction);
                }

                _physObjects[i].push_back(pObj);
            }
            else
                _physObjects[i].push_back(nullptr);
        }
    }
}
#include "MemoryLoggerOn.h"

int MultiSceneManager::getSceneIndex(interface::Scene* sc) const
{
    for(size_t i=0 ; i<_scenes.size() ; ++i)
    {
        if(_scenes[i].second ==  sc)
            return static_cast<int>(i);
    }

    return -1;
}

interface::View& MultiSceneManager::dirLightView(int index)
{
    return *_dirLightView[index];
}

void MultiSceneManager::buildLevels(LevelSystem& syst)
{
    for(size_t i=0 ; i<_scenes.size() ; ++i)
    {
        LevelSystem::Level lvl;
        lvl.name = _scenes[i].first;
        lvl.levelScene = _scenes[i].second;
        lvl.indexScene = (int)i;
        lvl.objects = _objects[i];
        lvl.physObjects = _physObjects[i];

        syst.addLevel(lvl);
    }
}
