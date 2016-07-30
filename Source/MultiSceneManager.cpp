#include "MultiSceneManager.h"
#include "resource/AssetManager.h"

using namespace interface;

MultiSceneManager::MultiSceneManager(std::string file, MultipleSceneHelper& multipleScene)
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

    std::map<std::string, EdgeInfo> nameEdge;
    int index=0;
    while(fs)
    {
        std::string sceneFile;
        std::getline(fs, sceneFile);

        interface::Scene* scene = new interface::Scene;
        vector<XmlSceneLoader::ObjectLoaded> objInScene;
        bool b = XmlSceneLoader::loadScene(sceneFile, *scene, objInScene);

        if(b)
        {
            sceneFile.resize(sceneFile.size()-3);
            _scenes.push_back({sceneFile, scene});
            _objects.push_back(objInScene);
            _dirLightView.push_back(new interface::View());

            if(index == 0)
            {
                multipleScene.pipeline().setScene(*scene, 0);
                multipleScene.pipeline().setDirLightView(*_dirLightView.back(), 0);

                multipleScene.setCurScene(*scene);
            }

            if(scene->globalLight.dirLights.size() > 0)
                _dirLightView.back()->dirLightView.lightDir = scene->globalLight.dirLights[0].direction;

            multipleScene.registerDirLightView(scene, _dirLightView.back());

            vector<GeometryShape::MeshInstance> staticGeom;
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
                                staticGeom.push_back({resource::AssetManager<Geometry>::instance().load<false>(obj.asset[i].geometry, true).value(),
                                                      obj.meshInstance->matrix()});
                            }
                        }
                    }

                    else if(obj.isPhysic && obj.isStatic)
                    {
                        for(XmlMeshAssetLoader::MeshElementModel part : obj.asset)
                        {
                            staticGeom.push_back({resource::AssetManager<Geometry>::instance().load<false>(part.geometry, true).value(),
                                                  obj.meshInstance->matrix()});
                        }
                    }
                }
            }

             btBvhTriangleMeshShape* shape = GeometryShape::genStaticGeometryShape(staticGeom);
             _staticGeom.push_back(shape);

            ++index;
        }
    }

    for(auto e : nameEdge)
    {
        if(e.second.toObj.empty())
            continue;

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

    for(auto ptr : _staticGeomObj)
        delete ptr;
}

void MultiSceneManager::instancePhysic(BulletEngine& bulletEngine)
{
    for(size_t i=0 ; i<_staticGeom.size() ; ++i)
    {
        if(i > 0)
            bulletEngine.createWorld(i);

        BulletObject* obj = new BulletObject(mat4::IDENTITY(), _staticGeom[i]);
        bulletEngine.addObject(obj, i);
        obj->body()->setRestitution(0.8);

        _staticGeomObj.push_back(obj);
    }
}

int MultiSceneManager::getSceneIndex(interface::Scene* sc) const
{
    for(size_t i=0 ; i<_scenes.size() ; ++i)
    {
        if(_scenes[i].second ==  sc)
            return static_cast<int>(i);
    }

    return -1;
}

