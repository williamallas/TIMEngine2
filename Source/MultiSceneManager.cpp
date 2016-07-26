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
            _scenes.push_back(scene);
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
                    }
                }
            }

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
        delete ptr;

    for(auto ptr : _dirLightView)
        delete ptr;
}

