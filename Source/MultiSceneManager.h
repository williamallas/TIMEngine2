#ifndef MULTISCENEMANAGER_H
#define MULTISCENEMANAGER_H

#include "MultipleSceneHelper.h"
#include "interface/XmlSceneLoader.h"

    class MultiSceneManager
    {
    public:
        MultiSceneManager(std::string, MultipleSceneHelper&);
        virtual ~MultiSceneManager();

    private:
        vector<interface::Scene*> _scenes;
        vector<interface::View*> _dirLightView;
        vector<vector<interface::XmlSceneLoader::ObjectLoaded>> _objects;

        //vector<std::pair<interface::MeshInstance*,interface::MeshInstance*>> _edges;
    };

#endif // MULTISCENEMANAGER_H
