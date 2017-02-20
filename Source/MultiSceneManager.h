#ifndef MULTISCENEMANAGER_H
#define MULTISCENEMANAGER_H

#include "MultipleSceneHelper.h"
#include "interface/XmlSceneLoader.h"
#include "bullet/GeometryShape.h"
#include "bullet/BulletEngine.h"
#include "PortalGame/LevelSystem.h"

    class MultiSceneManager
    {
    public:
        MultiSceneManager(std::string, MultipleSceneHelper&, int);
        virtual ~MultiSceneManager();

        void instancePhysic(BulletEngine&);
        void buildLevels(LevelSystem&);

        int getSceneIndex(interface::Scene*) const;
        interface::View& dirLightView(int);

    private:
        vector<std::pair<std::string, interface::Scene*>> _scenes;
        vector<interface::View*> _dirLightView;
        vector<vector<interface::XmlSceneLoader::ObjectLoaded>> _objects;
        vector<vector<BulletObject*>> _physObjects;

        vector<btBvhTriangleMeshShape*> _staticGeom;
        vector<btBvhTriangleMeshShape*> _staticRoomGeom;
        vector<BulletObject*> _staticGeomObj;
        vector<BulletObject*> _staticRoomGeomObj;

        std::map<vec3, btBoxShape*> _boxShapes;
        std::map<float, btSphereShape*> _sphereShapes;

        //vector<std::pair<interface::MeshInstance*,interface::MeshInstance*>> _edges;
    };

#endif // MULTISCENEMANAGER_H
