#include "PortalGame.h"
#include "resource/AssetManager.h"
#include "openAL/Source.hpp"

using namespace resource;

#include "MemoryLoggerOn.h"

PortalGame::PortalGame(BulletEngine& phys, MultipleSceneHelper& multiscene, HmdSceneView& hmdCam, VR_Device& vrdevice)
    : _physEngine(phys), _multiSceneHelper(multiscene), _hmdCamera(hmdCam), _vrDevice(vrdevice),
      _multiScene("configScene.txt", _multiSceneHelper), _vrControllers(phys), _levels(phys, _listener, _vrControllers, _hmdCamera)
{
    _multiScene.instancePhysic(_physEngine);

    _listener.initialize();
    _soundEffects.resize(SoundEffects::NB_EFFECTS);
    _soundEffects[SoundEffects::WOOD1] = AssetManager<SoundAsset>::instance().load<false>("soundBank/wood1.wav", false, Sampler::NONE).value();
    _soundEffects[SoundEffects::WOOD2] = AssetManager<SoundAsset>::instance().load<false>("soundBank/wood2.wav", false, Sampler::NONE).value();
    _soundEffects[SoundEffects::WOOD3] = AssetManager<SoundAsset>::instance().load<false>("soundBank/wood3.wav", false, Sampler::NONE).value();
    _soundEffects[SoundEffects::PLASTIC1] = AssetManager<SoundAsset>::instance().load<false>("soundBank/plastic1.wav", false, Sampler::NONE).value();
    _soundEffects[SoundEffects::PLASTIC2] = AssetManager<SoundAsset>::instance().load<false>("soundBank/plastic2.wav", false, Sampler::NONE).value();
    _soundEffects[SoundEffects::METAL1] = AssetManager<SoundAsset>::instance().load<false>("soundBank/metal1.wav", false, Sampler::NONE).value();
    _soundEffects[SoundEffects::METAL2] = AssetManager<SoundAsset>::instance().load<false>("soundBank/metal2.wav", false, Sampler::NONE).value();
    _soundEffects[SoundEffects::ARTIFACT1] = AssetManager<SoundAsset>::instance().load<false>("soundBank/artifact.wav", false, Sampler::NONE).value();

    const auto TEXTURE_CONFIG = interface::Texture::genParam(true,true,true, 4);
    _gameAssets.load("gameAssets.xml");
    _vrControllers.setControllerMesh(_gameAssets.getMesh("controller", TEXTURE_CONFIG));
    _vrControllers.setControllerOffset(mat4::RotationX(toRad(-86.1672))*mat4::Translation({0, 0.121448f*0.6f, -0.020856f*0.6f}));
    _vrControllers.buildForScene(*_multiSceneHelper.curScene(), _multiScene.getSceneIndex(_multiSceneHelper.curScene()));

    _levels.setPortalHelper(&_multiSceneHelper);
    _multiScene.buildLevels(_levels);

    Sync_Ocean_FlyingIsland_PTR syncOceanFI = std::make_shared<Sync_Ocean_FlyingIsland>();

    for(int i=0 ; i<_levels.nbLevels() ; ++i)
    {
        if(_levels.getLevel(i).name == "forest1")
            _levels.setStrategy(new ForestLevel1(i, &_levels, _physEngine), i);
        else if(_levels.getLevel(i).name == "forest2")
            _levels.setStrategy(new ForestLevel2(i, &_levels, _physEngine), i);
        else if(_levels.getLevel(i).name == "forest3")
            _levels.setStrategy(new ForestLevel3(i, &_levels, _physEngine), i);
        else if(_levels.getLevel(i).name == "sacredGrove_red")
            _levels.setStrategy(new SacredGroveAux(i, &_levels, _physEngine, "red"), i);
        else if(_levels.getLevel(i).name == "sacredGrove_blue")
            _levels.setStrategy(new SacredGroveAux(i, &_levels, _physEngine, "blue"), i);
        else if(_levels.getLevel(i).name == "sacredGrove_white")
            _levels.setStrategy(new SacredGroveAux(i, &_levels, _physEngine, "white"), i);
        else if(_levels.getLevel(i).name == "sacredGrove" || _levels.getLevel(i).name == "sacredGrove_final")
            _levels.setStrategy(new SacredGroveMain(i, &_levels, _physEngine), i);
        else if(_levels.getLevel(i).name == "ocean")
            _levels.setStrategy(new OceanLevel(i, &_levels, _physEngine, syncOceanFI), i);
        else if(_levels.getLevel(i).name == "flyingIsland")
            _levels.setStrategy(new FlyingIslandLevel(i, &_levels, _physEngine, syncOceanFI), i);
        else
            _levels.setStrategy(new Level1(i, &_levels), i);
    }
    _levels.initAll();
    _levels.changeLevel(0);

    registerSoundCallBack();
}

void PortalGame::update(float time)
{
    /* First check if we have switched between 2 scenes */

    interface::Scene* switchScene = nullptr;
    mat4 o;
    if(_multiSceneHelper.update(switchScene, &o))
    {
        _hmdCamera.addOffset(o);
        //std::cout << "Offset:" << _hmdCamera.offset().translation() << std::endl;

        int index = _multiScene.getSceneIndex(switchScene);

         _vrControllers.buildForScene(*switchScene, index);
        _levels.changeLevel(index);
    }

    /* then update controllers */
    if (_vrDevice.isInit())
    {
        mat4 l = _vrDevice.isControllerConnected(VR_Device::LEFT) ? _vrDevice.controllerPose(VR_Device::LEFT) : _lastL;
        mat4 r = _vrDevice.isControllerConnected(VR_Device::RIGHT) ? _vrDevice.controllerPose(VR_Device::RIGHT) : _lastR;

        _vrControllers.update(_hmdCamera.offset(), l,r, time);
        _lastL = _hmdCamera.offset()*l;
        _lastR = _hmdCamera.offset()*r;
    }
    else
    {
        mat4 l = mat4::RotationY(45);
        l.setTranslation(_debugControllerPos);

        mat4 r = l;
        r.translate({0,0,0.3});
        _vrControllers.update(_hmdCamera.offset(), l,r, time);
        _lastL = _hmdCamera.offset()*l;
        _lastR = _hmdCamera.offset()*r;
    }

    /* instance controllers through portals if needed */

    mat4 offset;
    interface::Scene* sceneNext = _multiSceneHelper.closestPortal(Sphere(_lastL.translation(), 1), offset).first;
    if(!sceneNext)
        sceneNext = _multiSceneHelper.closestPortal(Sphere(_lastR.translation(), 1), offset).first;

    if(sceneNext)
        _vrControllers.buildSecondary(*sceneNext, _multiScene.getSceneIndex(sceneNext), offset * _hmdCamera.offset());
    else
        _vrControllers.removeSecondary();

    /* then update others stuffs */

    _levels.update(time);

    _listener.setTransform(_hmdCamera.transform());
    _listener.update();

    _multiScene.dirLightView(_levels.getCurLevelIndex()).dirLightView.camPos = _hmdCamera.cullingView().camera.pos;

    for(Source* src : _asyncSoundToPlay)
    {
        src->play();
        src->release();
    }
    _asyncSoundToPlay.clear();

    if(_clearSoundPairTimer++ % 20 == 0)
    {
        _lastSoundPair.clear();
        _nbTotalSound = 0;
    }
}

int PortalGame::curSceneIndex() const
{
    return _levels.getCurLevelIndex();
}

void constructCollidePaddleObj(btDiscreteDynamicsWorld* world, std::set<const btCollisionObject*> sets[])
{
    int numManifolds = world->getDispatcher()->getNumManifolds();
    for (int i=0 ; i<numManifolds ; ++i)
    {
        btPersistentManifold* contactManifold =  world->getDispatcher()->getManifoldByIndexInternal(i);
        const btCollisionObject* obA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
        const btCollisionObject* obB = static_cast<const btCollisionObject*>(contactManifold->getBody1());

        if(obA->getUserIndex() >= 100 || obB->getUserIndex() >= 100)
        {
           if(obA->getUserIndex() == 90 || obA->getUserIndex() == 91)
               sets[obA->getUserIndex()-90].insert(obB);
           if(obB->getUserIndex() == 90 || obB->getUserIndex() == 91)
               sets[obB->getUserIndex()-90].insert(obA);
        }
    }
}

bool PortalGame::processContactPoint(btManifoldPoint& pt, const btCollisionObject* obj, btVector3 posContact)
{
    const float SOUND_THRESHOLD = 0.3;
    const float MAX_STRENGTH = 1.3;
    float f = pt.getAppliedImpulse();
    const btRigidBody* b = dynamic_cast<const btRigidBody*>(obj);
    if(b) f *= b->getInvMass();

    float strength = std::min(MAX_STRENGTH, f);
    float norm_strength = (strength - SOUND_THRESHOLD) / (MAX_STRENGTH - SOUND_THRESHOLD);

    if(f > SOUND_THRESHOLD)
    {
        Source* s = _listener.addSource(_soundEffects[obj->getUserIndex()-100]);
        s->setPosition(vec3(posContact.x(), posContact.y(), posContact.z()));
        s->setGain(strength);
        //s->setPitch(1.5f - norm_strength);
        s->setPitch(0.6 + norm_strength*0.85);
        _asyncSoundToPlay.push_back(s);

        return true;
    }
    return false;
}

void PortalGame::registerSoundCallBack()
{
for(int index=0 ; index < _levels.nbLevels() ; ++index)
    _physEngine.addInnerPhysicTask([this, index](float, BulletEngine*){        
        btDiscreteDynamicsWorld* world = _physEngine.dynamicsWorld[index];

        std::set<const btCollisionObject*> objCollidePaddle[2];
        constructCollidePaddleObj(world, objCollidePaddle);

        if(_vrControllers.getAppliedStrength(0) < 100)
            objCollidePaddle[0].clear();
        if(_vrControllers.getAppliedStrength(1) < 100)
            objCollidePaddle[1].clear();

        int numManifolds = world->getDispatcher()->getNumManifolds();
        for (int i=0 ; i<numManifolds ; ++i)
        {
            btPersistentManifold* contactManifold =  world->getDispatcher()->getManifoldByIndexInternal(i);
            const btCollisionObject* obA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
            const btCollisionObject* obB = static_cast<const btCollisionObject*>(contactManifold->getBody1());

            int numContacts = contactManifold->getNumContacts();
            if(obA->getUserIndex() >= 100 || obB->getUserIndex() >= 100)
            {
                if(objCollidePaddle[0].find(obA) != objCollidePaddle[0].end() || objCollidePaddle[1].find(obA) != objCollidePaddle[1].end() ||
                   objCollidePaddle[0].find(obB) != objCollidePaddle[0].end() || objCollidePaddle[1].find(obB) != objCollidePaddle[1].end())
                        continue;

                for (int j = 0 ; j<numContacts ; j++)
                {
                    btManifoldPoint& pt = contactManifold->getContactPoint(j);
                    //if(pt.getDistance() < 0.f)
                    {
                        if(pt.getLifeTime() == 1)
                        {
                            auto p = (obA<obB ? std::pair<const btCollisionObject*,const btCollisionObject*>(obA,obB) :
                                                std::pair<const btCollisionObject*,const btCollisionObject*>(obB,obA));

                            if(_lastSoundPair[p] < 1 || _nbTotalSound < 4)
                            {
                                bool played = false;
                                if(obA->getUserIndex() >= 100)
                                    played = played || processContactPoint(pt, obA, pt.getPositionWorldOnA());
                                if(obB->getUserIndex() >= 100)
                                    played = played || processContactPoint(pt, obB, pt.getPositionWorldOnB());

                                if(played)
                                {
                                    _nbTotalSound++;
                                    _lastSoundPair[p]++;
                                }
                            }
                        }
                    }
                }
            }
        }

    }, index);
}

#include "PortalGame/CollisionMask.h"

void PortalGame::popBoxDebug()
{
    static btBoxShape b_shape(btVector3(0.25,0.25,0.25));
    interface::MeshInstance& m = _levels.getLevel(_levels.getCurLevelIndex()).levelScene->scene.add<interface::MeshInstance>(mat4::Translation(_lastL.translation() - vec3(0,0,0.5)));
    m.setMesh(_gameAssets.getMesh("caisse", interface::Texture::genParam(true,true,true, 4)));

    BulletObject* bo = new BulletObject(new SceneMotionState<interface::MeshInstance>(m, 1), &b_shape, 0.5);

    _physEngine.addObject(bo, _levels.getCurLevelIndex(), COL_PHYS, PHYS_COLLISION);
}

