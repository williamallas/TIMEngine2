#include "ForestLevel.h"
#include "resource/AssetManager.h"
#include "bullet/BulletObject.h"
#include "openAL/Source.hpp"
#include "PortalGame.h"
#include "Rand.h"

#include "MemoryLoggerOn.h"


ForestLevelBase::ForestLevelBase(int index, LevelSystem* system, BulletEngine& phys) : LevelInterface(index, system), _physEngine(phys)
{
    _birds = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/birds1.wav", false, Sampler::NONE).value();

    resource::SoundAsset ambientSound = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/addressing_stars.ogg", true, Sampler::NONE).value();
    Source* src = system->listener().addSource(ambientSound);
    src->setLooping(true);
    src->setGain(0.1);
    setAmbientSound(src, "addressing_stars");
}

void ForestLevelBase::init()
{
    for(size_t i=0 ; i<level().objects.size() ; ++i)
    {
        if(level().physObjects[i] && level().objects[i].model == "caisse")
        {
            BulletObject* bo = level().physObjects[i];
            bo->body()->setCollisionFlags(bo->body()->getCollisionFlags() |
                                          btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

            bo->body()->setUserIndex(PortalGame::SoundEffects::WOOD3 + 100);
        }

        else if(level().physObjects[i] && (level().objects[i].model == "kapla1" || level().objects[i].model == "kapla2"))
        {
            BulletObject* bo = level().physObjects[i];
            bo->body()->setCollisionFlags(bo->body()->getCollisionFlags() |
                                          btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

            bo->body()->setUserIndex(PortalGame::SoundEffects::WOOD1 + 100);
        }
    }
}

void ForestLevelBase::update(float time)
{
    static float birdAccumulator = 0;
    birdAccumulator += time;
    if(birdAccumulator > 5)
    {
        birdAccumulator = 0;
        if(Rand::rand() % 4 == 0)
        {
            Source* src = levelSystem().listener().addSource(_birds);

            vec3 p(Rand::frand() * 30 - 15, Rand::frand() * 30 - 15, 2 + Rand::frand()*8);
            if(p.to<2>().length2() < 40)
                p.resize(15);

            p += levelSystem().listener().transform().translation();

            src->setPosition(p);
            src->play();
            src->release();
        }
    }
}


ForestLevel1::ForestLevel1(int index, LevelSystem* system, BulletEngine& phys) : ForestLevelBase(index, system, phys)
{
    _sunTexture = resource::AssetManager<interface::Texture>::instance().load<false>("textureBank/sun2.png", LevelSystem::defaultTexParam).value();
}

void ForestLevel1::init()
{
    ForestLevelBase::init();
    _indexPortal = indexObject("portalForest1_Forest2");
    if(_indexPortal < 0)
        LOG("portalForest1_Forest2 in scene1 not found");
    else
        setEnablePortal(false, level().objects[_indexPortal].meshInstance);

    int index = indexObject("sunStone");
    if(index >= 0)
        _instSunStone = level().objects[index].meshInstance;
    else
    {
        _instSunStone = nullptr;
        LOG("In forest1, no sunStone found");
    }
}

void ForestLevel1::update(float time)
{
    ForestLevelBase::update(time);

    BulletObject::CollisionPoint p;
    vec3 stp = _instSunStone->matrix().translation() + vec3(0,0,0.05);
    bool allOk = !BulletObject::rayCastFirst(stp, stp - level().levelScene->globalLight.dirLights[0].direction * 30,
                                             p, *_physEngine.dynamicsWorld[index()]);

    if(allOk && _first)
    {
        setEnablePortal(true, level().objects[_indexPortal].meshInstance);

        int index = indexObject("sunStone");

        if(index >= 0)
        {
            interface::MeshInstance* inst = level().objects[index].meshInstance;
            interface::Mesh m = inst->mesh();
            m.element(0).setTexture(_sunTexture, 0);
            inst->setMesh(m);
        }

        _first = false;
    }
    else if(!_first)
    {
        setEnablePortal(true, level().objects[_indexPortal].meshInstance);
    }
}

ForestLevel2::ForestLevel2(int index, LevelSystem* system, BulletEngine& phys) : ForestLevelBase(index, system, phys)
{
    _sunTexture1[0] = resource::AssetManager<interface::Texture>::instance().load<false>("textureBank/sun.png", LevelSystem::defaultTexParam).value();
    _sunTexture1[1] = resource::AssetManager<interface::Texture>::instance().load<false>("textureBank/sun2.png", LevelSystem::defaultTexParam).value();

    _sunTexture2[0] = resource::AssetManager<interface::Texture>::instance().load<false>("meshBank/rock6_st2.png", LevelSystem::defaultTexParam).value();
    _sunTexture2[1] = resource::AssetManager<interface::Texture>::instance().load<false>("meshBank/rock6_st1.png", LevelSystem::defaultTexParam).value();
}

void ForestLevel2::init()
{
    ForestLevelBase::init();

    int index = indexObject("sunStone1");
    if(index >= 0)
        _sunStone[0] = level().objects[index].meshInstance;

    vector<int> sts = indexObjects("sunStone2");
    for(size_t i=0 ; i<std::max(sts.size(), 4u) ; ++i)
        if(sts[i] >= 0)
            _sunStone[i+1] = level().objects[sts[i]].meshInstance;

    sts = indexObjects("sunStone3");
    for(size_t i=0 ; i<std::max(sts.size(), 2u) ; ++i)
        if(sts[i] >= 0)
            _sunStone[i+5] = level().objects[sts[i]].meshInstance;
}

void ForestLevel2::update(float time)
{
    ForestLevelBase::update(time);

    if(_updateRate++ % 30 != 0)
        return;

    bool state[7] = {true};
    for(int i=0 ; i<7 ; ++i)
    {
        if(!_sunStone[i]) continue;

        BulletObject::CollisionPoint p_tmp;
        vec3 p;
        if(i == 0)
            p = _sunStone[i]->matrix().translation() + vec3(-0.05,0,0);
        else
            p = _sunStone[i]->matrix().translation() + vec3(0,0,0.95);

        state[i] = BulletObject::rayCastFirst(p, p - level().levelScene->globalLight.dirLights[0].direction * 30,
                                              p_tmp, *_physEngine.dynamicsWorld[index()]);

        interface::Mesh m = _sunStone[i]->mesh();
        if(i == 0)
            m.element(0).setTexture(_sunTexture1[state[i] ? 0:1], 0);
        else
            m.element(0).setTexture(_sunTexture2[state[i] ? 0:1], 0);
        _sunStone[i]->setMesh(m);
    }

    if(state[0] && !state[1] && !state[2] && !state[3] && !state[4] && state[5] && state[6])
        std::cout << "NICE\n";
}

ForestLevel3::ForestLevel3(int index, LevelSystem* system, BulletEngine& phys) : ForestLevelBase(index, system, phys)
{
    _warp = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/warp.wav", false, Sampler::NONE).value();
}

void ForestLevel3::init()
{
    ForestLevelBase::init();

    _indexArtifact = indexObject("artifact");
    _indexSlot = indexObject("artifactSlot");
    _indexPortal = indexObject("portalForest3Out");
    _indexArtifactInPlace = indexObject("artifactInPlace");

    if(_indexArtifact >= 0)
    {
        registerPortableTraversable(_indexArtifact, level().objects[_indexArtifact].meshInstance, level().physObjects[_indexArtifact]);
        _nonActivatedArtifactMesh = level().objects[_indexArtifact].meshInstance->mesh();
    }
}

void ForestLevel3::update(float time)
{
    ForestLevelBase::update(time);

    if(_indexArtifact >= 0 && _indexSlot >= 0)
    {
        static float timeOnSlot = 0;
        static bool justInPlace = false;

        btTransform tr = level().physObjects[_indexArtifact]->body()->getWorldTransform();
        vec3 pos(tr.getOrigin().getX(), tr.getOrigin().getY(), tr.getOrigin().getZ());
        vec3 posSlot = level().objects[_indexSlot].meshInstance->matrix().translation();
        float dist = (pos - posSlot - vec3(0,0,1.2)).length();

        if(dist < 0.2)
            timeOnSlot += time;
        else
            timeOnSlot = 0;

        if(level().physObjects[_indexArtifact]->body()->getLinearVelocity().length() > 0.1)
            timeOnSlot = 0;

        if(timeOnSlot > 2 && _indexArtifactInPlace >= 0)
        {
            level().objects[_indexArtifact].meshInstance->setMesh(level().objects[_indexArtifactInPlace].meshInstance->mesh());

            if(!justInPlace)
            {
                justInPlace = true;
                Source* src = levelSystem().listener().addSource(_warp);
                src->setPosition({14.069, 15.393, 1.5});
                src->play();
                src->release();
            }
        }
        else
        {
            justInPlace = false;
            level().objects[_indexArtifact].meshInstance->setMesh(_nonActivatedArtifactMesh);
        }
    }
}
