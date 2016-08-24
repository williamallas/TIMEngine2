#ifndef CONTROLLER_H__
#define CONTROLLER_H__

#include "interface/Pipeline.h"
#include "interface/pipeline/pipeline.h"
#include "OpenVR/VR_Device.h"
#include "bullet/BulletEngine.h"

using namespace tim;

class Controller
{
    public:
        struct InScene
        {
            interface::Scene* scene = nullptr;
            interface::MeshInstance* leftHand = nullptr;
            interface::MeshInstance* rightHand = nullptr;

            BulletObject* leftHandPhys = nullptr;
            BulletObject* rightHandPhys = nullptr;
            mat4 absoluteOffset = mat4::IDENTITY();
        };

        Controller(interface::Mesh, BulletEngine&);
        Controller(BulletEngine&);
        ~Controller();

        void setControllerMesh(const interface::Mesh& mesh) { _controllerMesh = mesh; }

        void update(const mat4& offset, const mat4& l, const mat4& r, float time);
        void buildForScene(interface::Scene&, int worldIndex);
        void buildSecondary(interface::Scene&, int worldIndex, const mat4& offset);
        void removeSecondary();

        vec3 avgControllersPos() const;
        float getAppliedStrength(int hand) const { return _strengthApplied[hand]; }

        void setControllerOffset(const mat4& o) { _controllerOffset = o; }

        const InScene& controllerInfo() { return _curScene; }

        float STRENGTH = 12000, DAMPING = 150;
        float STRENGTH_R = 60, DAMPING_R = 1;

    private:
        BulletEngine& _bullet;

        interface::MeshInstance* debugController[2] = {nullptr,nullptr};

        InScene _curScene;
        InScene _lastScene;

        interface::Mesh _controllerMesh;
        mat4 _controllerOffset;
        float _mass = 0.5;
        float _strengthApplied[2];

        btCollisionShape* _controllerShape = nullptr;

        void clear(InScene&);

        void innerPhysicUpdate(float time, BulletEngine*);
        void updateBody(btRigidBody*, const mat4&, int);

};

#endif // CONTROLLER_H
