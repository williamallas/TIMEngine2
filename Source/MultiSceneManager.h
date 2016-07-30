#ifndef MULTISCENEMANAGER_H
#define MULTISCENEMANAGER_H

#include "MultipleSceneHelper.h"
#include "interface/XmlSceneLoader.h"
#include "bullet/GeometryShape.h"
#include "bullet/BulletEngine.h"

    class MultiSceneManager
    {
    public:
        MultiSceneManager(std::string, MultipleSceneHelper&);
        virtual ~MultiSceneManager();

        void instancePhysic(BulletEngine&);

        int getSceneIndex(interface::Scene*) const;

    private:
        vector<std::pair<std::string, interface::Scene*>> _scenes;
        vector<interface::View*> _dirLightView;
        vector<vector<interface::XmlSceneLoader::ObjectLoaded>> _objects;

        vector<btBvhTriangleMeshShape*> _staticGeom;
        vector<BulletObject*> _staticGeomObj;

        //vector<std::pair<interface::MeshInstance*,interface::MeshInstance*>> _edges;
    };

#endif // MULTISCENEMANAGER_H
