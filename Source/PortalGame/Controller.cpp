#include "Controller.h"

Controller::Controller(interface::Mesh controllerMesh, BulletEngine& bulletEngine)
    : _controllerMesh(controllerMesh), _bullet(bulletEngine)
{
    _controllerShape = new btBoxShape(btVector3(0.096071, 0.096071, 0.012231));
}

Controller::~Controller()
{
    delete _controllerShape;
    clear(_curScene);
    clear(_lastScene);
}

void Controller::clear(InScene& sc)
{
    if(sc.scene && sc.leftHand)
        sc.scene->scene.remove(*sc.leftHand);

    if(sc.scene && sc.rightHand)
        sc.scene->scene.remove(*sc.rightHand);

    delete sc.leftHandPhys;
    delete sc.rightHandPhys;
}

void Controller::buildForScene(interface::Scene& scene, int worldIndex)
{
    clear(_lastScene);
    _lastScene = _curScene;

    _curScene.leftHand = &scene.scene.add<interface::MeshInstance>(_controllerMesh, mat4::IDENTITY());
    _curScene.rightHand = &scene.scene.add<interface::MeshInstance>(_controllerMesh, mat4::IDENTITY());
    _curScene.scene = &scene;

    _curScene.leftHandPhys = new BulletObject(new SceneMotionState<interface::MeshInstance>(*_curScene.leftHand), _controllerShape, 0);
    _curScene.rightHandPhys = new BulletObject(new SceneMotionState<interface::MeshInstance>(*_curScene.rightHand), _controllerShape, 0);

    _bullet.addObject(_curScene.leftHandPhys, worldIndex);
    _bullet.addObject(_curScene.rightHandPhys, worldIndex);

    _curScene.leftHandPhys->body()->setFriction(1);
    _curScene.leftHandPhys->body()->setRestitution(0.8);

    _curScene.rightHandPhys->body()->setFriction(1);
    _curScene.rightHandPhys->body()->setRestitution(0.8);

    _curScene.leftHandPhys->body()->setCollisionFlags(_curScene.leftHandPhys->body()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    _curScene.leftHandPhys->body()->setActivationState(DISABLE_DEACTIVATION);

    _curScene.rightHandPhys->body()->setCollisionFlags(_curScene.rightHandPhys->body()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    _curScene.rightHandPhys->body()->setActivationState(DISABLE_DEACTIVATION);
}

void Controller::update(const mat4& offset, const mat4& l, const mat4& r)
{
    _curScene.absoluteOffset = offset;

    if(_curScene.leftHand)
        _curScene.leftHand->setMatrix(offset * l * _controllerOffset);
    if(_curScene.rightHand)
        _curScene.rightHand->setMatrix(offset * r * _controllerOffset);

    if(_lastScene.leftHand)
        _lastScene.leftHand->setMatrix(_lastScene.absoluteOffset * l * _controllerOffset);
    if(_lastScene.rightHand)
        _lastScene.rightHand->setMatrix(_lastScene.absoluteOffset * r * _controllerOffset);

}

