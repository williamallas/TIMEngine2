#include "Controller.h"
#include "resource/AssetManager.h"

#include "MemoryLoggerOn.h"
Controller::Controller(interface::Mesh controllerMesh, BulletEngine& bulletEngine)
    : _bullet(bulletEngine), _controllerMesh(controllerMesh)
{
    #include "MemoryLoggerOff.h"
    _controllerShape = new btBoxShape(btVector3(0.096071, 0.096071, 0.012231));
    #include "MemoryLoggerOn.h"

    bulletEngine.addInnerPhysicTask([=](float time, BulletEngine* engine) {
        this->innerPhysicUpdate(time, engine);
    }, -1);
}

Controller::Controller(BulletEngine& bulletEngine)
    : _bullet(bulletEngine)
{
    #include "MemoryLoggerOff.h"
    _controllerShape = new btBoxShape(btVector3(0.096071, 0.096071, 0.012231));
    #include "MemoryLoggerOn.h"

    bulletEngine.addInnerPhysicTask([=](float time, BulletEngine* engine) {
        this->innerPhysicUpdate(time, engine);
    }, -1);
}

Controller::~Controller()
{
    delete _controllerShape;
    clear(_curScene);
    clear(_lastScene);
}

vec3 Controller::avgControllersPos() const
{
    if(_curScene.leftHand && _curScene.rightHand)
    {
        return _curScene.leftHand->matrix().translation()*0.5 +
               _curScene.rightHand->matrix().translation()*0.5;
    }
    return vec3(0,0,0);
}

void Controller::clear(InScene& sc)
{
    if(sc.scene && sc.leftHand)
        sc.scene->scene.remove(*sc.leftHand);

    if(sc.scene && sc.rightHand)
        sc.scene->scene.remove(*sc.rightHand);

    delete sc.leftHandPhys;
    delete sc.rightHandPhys;

    sc = InScene();
}

void configureControllerBody(BulletObject* obj)
{
    obj->body()->setFriction(2);
    obj->body()->setRestitution(0.8);

    //obj->body()->setCollisionFlags(_curScene.leftHandPhys->body()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    obj->body()->setActivationState(DISABLE_DEACTIVATION);
    obj->body()->setGravity(btVector3(0,0,0));
}

void Controller::buildForScene(interface::Scene& scene, int worldIndex)
{
    clear(_lastScene);
    _lastScene = _curScene;


    _curScene.leftHand = &scene.scene.add<interface::MeshInstance>(_controllerMesh, mat4::Translation(vec3(0,0,2)));
    _curScene.rightHand = &scene.scene.add<interface::MeshInstance>(_controllerMesh, mat4::Translation(vec3(0,0,2)));
    _curScene.scene = &scene;

//    interface::Geometry dg = resource::AssetManager<interface::Geometry>::instance().load<false>("raquette_shape_simple.obj").value();
//    debugController[0] = &scene.scene.add<interface::MeshInstance>(_controllerMesh, mat4::Translation(vec3(0,0,2)));
//    debugController[1] = &scene.scene.add<interface::MeshInstance>(_controllerMesh, mat4::Translation(vec3(0,0,2)));

//    _curScene.leftHandPhys = new BulletObject(new SceneMotionState<interface::MeshInstance>(*debugController[0]), _controllerShape, _mass);
//    _curScene.rightHandPhys = new BulletObject(new SceneMotionState<interface::MeshInstance>(*debugController[1]), _controllerShape, _mass);

//    _curScene.leftHandPhys = new BulletObject(new SceneMotionState<interface::MeshInstance>(*_curScene.leftHand), _controllerShape, _mass);
//    _curScene.rightHandPhys = new BulletObject(new SceneMotionState<interface::MeshInstance>(*_curScene.rightHand), _controllerShape, _mass);

    #include "MemoryLoggerOff.h"
     _curScene.leftHandPhys = new BulletObject(new btDefaultMotionState(), _controllerShape, _mass);
     _curScene.rightHandPhys = new BulletObject(new btDefaultMotionState(), _controllerShape, _mass);
    #include "MemoryLoggerOn.h"

    _bullet.addObject(_curScene.leftHandPhys, worldIndex);
    _bullet.addObject(_curScene.rightHandPhys, worldIndex);

    configureControllerBody(_curScene.leftHandPhys);
    configureControllerBody(_curScene.rightHandPhys);

    _curScene.leftHandPhys->body()->setUserIndex(90);
    _curScene.rightHandPhys->body()->setUserIndex(91);
}

void Controller::buildSecondary(interface::Scene& scene, int worldIndex, const mat4& offset)
{
    _lastScene.absoluteOffset = offset;

    if(&scene == _lastScene.scene)
        return;

    clear(_lastScene);

    _lastScene.leftHand = &scene.scene.add<interface::MeshInstance>(_controllerMesh, mat4::Translation(vec3(0,0,2)));
    _lastScene.rightHand = &scene.scene.add<interface::MeshInstance>(_controllerMesh, mat4::Translation(vec3(0,0,2)));
    _lastScene.scene = &scene;
    _lastScene.absoluteOffset = offset;

    #include "MemoryLoggerOff.h"
    _lastScene.leftHandPhys = new BulletObject(new btDefaultMotionState(), _controllerShape, _mass);
    _lastScene.rightHandPhys = new BulletObject(new btDefaultMotionState(), _controllerShape, _mass);
    #include "MemoryLoggerOn.h"

   _bullet.addObject(_lastScene.leftHandPhys, worldIndex);
   _bullet.addObject(_lastScene.rightHandPhys, worldIndex);

   configureControllerBody(_lastScene.leftHandPhys);
   configureControllerBody(_lastScene.rightHandPhys);

   _lastScene.leftHandPhys->body()->setUserIndex(90);
   _lastScene.rightHandPhys->body()->setUserIndex(91);
}

void Controller::removeSecondary()
{
    clear(_lastScene);
}

btVector3 QuaternionToEulerXYZ(const btQuaternion &quat)
{
   float w=quat.getW();   float x=quat.getX();   float y=quat.getY();   float z=quat.getZ();
   double sqw = w*w; double sqx = x*x; double sqy = y*y; double sqz = z*z;
   btVector3 euler;
   euler.setZ((atan2(2.0 * (x*y + z*w),(sqx - sqy - sqz + sqw))));
   euler.setX((atan2(2.0 * (y*z + x*w),(-sqx - sqy + sqz + sqw))));
   euler.setY((asin(-2.0 * (x*z - y*w))));
   return euler;
}

void Controller::update(const mat4& offset, const mat4& l, const mat4& r, float time)
{
    _curScene.absoluteOffset = offset;

    if(_curScene.leftHand)
    {
        _curScene.leftHand->setMatrix(offset * l * _controllerOffset);
    }
    if(_curScene.rightHand)
    {
        _curScene.rightHand->setMatrix(offset * r * _controllerOffset);
    }

    if(_lastScene.leftHand)
    {
        _lastScene.leftHand->setMatrix(_lastScene.absoluteOffset * l * _controllerOffset);
    }
    if(_lastScene.rightHand)
    {
        _lastScene.rightHand->setMatrix(_lastScene.absoluteOffset * r * _controllerOffset);
    }
}

void Controller::updateBody(btRigidBody* body, const mat4& o, int hand)
{
    body->clearForces();

    btVector3 deltaLinear = btVector3(o.translation().x(), o.translation().y(), o.translation().z()) - body->getWorldTransform().getOrigin();

    if(deltaLinear.length() > 0.3)
        body->setWorldTransform(BulletObject::toBtTransform(o));
    else
    {
        //const float STRENGH = 20000;
        //const float DAMPING = 1000;

        btVector3 v = body->getLinearVelocity();

        btVector3 capOr = deltaLinear;
        if(capOr.length() > 0.3)
            capOr = capOr.normalized() * 0.3;

        btVector3 str = capOr * STRENGTH * _mass - v*DAMPING*_mass;
        body->applyCentralForce(str);
        _strengthApplied[hand] = std::max(_strengthApplied[hand], str.length());

        btQuaternion t1 = body->getWorldTransform().getRotation().inverse();
        btQuaternion t2 = BulletObject::toBtTransform(o).getRotation();
        btQuaternion delta = t2*t1;

        v = body->getAngularVelocity();

        capOr = QuaternionToEulerXYZ(delta);

        if(capOr.length() > 1)
            capOr = capOr.normalized() * 1;

        str = capOr*STRENGTH_R*_mass  - v*DAMPING_R*_mass;
        body->applyTorque(str);
        _strengthApplied[hand] = std::max(_strengthApplied[hand], str.length());
    }
}

void Controller::innerPhysicUpdate(float time, BulletEngine* engine)
{
    _strengthApplied[0] = 0;
    _strengthApplied[1] = 1;

    if(_curScene.leftHandPhys && _curScene.leftHand)
    {
        updateBody(_curScene.leftHandPhys->body(), _curScene.leftHand->matrix(), 0);
    }

    if(_curScene.rightHandPhys && _curScene.rightHand)
    {
        updateBody(_curScene.rightHandPhys->body(), _curScene.rightHand->matrix(), 1);
    }

    if(_lastScene.leftHandPhys && _lastScene.leftHand)
    {
        updateBody(_lastScene.leftHandPhys->body(), _lastScene.leftHand->matrix(), 0);
    }

    if(_lastScene.rightHandPhys && _lastScene.rightHand)
    {
        updateBody(_lastScene.rightHandPhys->body(), _lastScene.rightHand->matrix(), 1);
    }
}
