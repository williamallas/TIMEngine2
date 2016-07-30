#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "interface/Pipeline.h"
#include "interface/pipeline/pipeline.h"
#include "OpenVR/VR_Device.h"
#include "bullet/BulletEngine.h"

using namespace tim;

class Controller
{
    public:
        Controller(interface::Mesh, BulletEngine&);
        ~Controller();

        void update(const mat4& offset, const mat4& l, const mat4& r);
        void buildForScene(interface::Scene&, int worldIndex);

        void setControllerOffset(const mat4& o) { _controllerOffset = o; }

    private:
        BulletEngine& _bullet;

        struct InScene
        {
            interface::Scene* scene = nullptr;
            interface::MeshInstance* leftHand = nullptr;
            interface::MeshInstance* rightHand = nullptr;

            BulletObject* leftHandPhys = nullptr;
            BulletObject* rightHandPhys = nullptr;
            mat4 absoluteOffset = mat4::IDENTITY();
        };

        InScene _curScene;
        InScene _lastScene;

        interface::Mesh _controllerMesh;
        mat4 _controllerOffset;

        btCollisionShape* _controllerShape = nullptr;

        void clear(InScene&);

};

#endif // CONTROLLER_H
