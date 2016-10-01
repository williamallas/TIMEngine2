#include "OceanLevel.h"
#include "resource/AssetManager.h"
#include "bullet/BulletObject.h"
#include "openAL/Source.hpp"
#include "PortalGame.h"
#include "Rand.h"

#include "MemoryLoggerOn.h"

OceanLevel::OceanLevel(int index, LevelSystem* system, BulletEngine& phys) : LevelInterface(index, system), _physEngine(phys)
{
    _buttonSound = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/pheub.wav", false, Sampler::NONE).value();
    _warpSound = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/warp.wav", false, Sampler::NONE).value();

    //_ambientOcean = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/ocean.ogg", true, Sampler::NONE).value();
    _ambientOceanSource = system->listener().addSource(resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/ocean.ogg", true, Sampler::NONE).value());
    _ambientOceanSource->setLooping(true);
    _ambientOceanSource->setGain(0.05);

    resource::SoundAsset ambientSound = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/NoWinners.ogg", true, Sampler::NONE).value();
    Source* src = system->listener().addSource(ambientSound);
    src->setLooping(true);
    src->setGain(0.1);
    setAmbientSound(src, "NoWinners");
}

OceanLevel::~OceanLevel()
{
    _ambientOceanSource->release();
}

void OceanLevel::init()
{
    _artifactIndex = indexObject("oceanArtifact");
    if(_artifactIndex >= 0)
    {
        registerGameObject(_artifactIndex, "oceanArtifact");

        vector<std::string> forbiddenPortals(4);
        forbiddenPortals[0] = "portalOcean12_Ocean13";
        forbiddenPortals[1] = "portalOcean13_Ocean12";
        forbiddenPortals[2] = "portalOcean14_Ocean15";
        forbiddenPortals[3] = "portalOcean15_Ocean14";
        registerPortableTraversable(_artifactIndex, level().objects[_artifactIndex].meshInstance, level().physObjects[_artifactIndex], forbiddenPortals);
    }

    SlotArtifact slot0 = createSlotArtifact("slotArtifact0", "reset0", "");
    if(slot0.inst)
        _slots.push_back(slot0);

    SlotArtifact slot1 = createSlotArtifact("slotArtifact1", "reset1", "portalOcean2_Ocean3");
    if(slot1.inst)
        _slots.push_back(slot1);

    SlotArtifact slot2 = createSlotArtifact("slotArtifact2", "reset2", "portalOcean4_Ocean5");
    if(slot2.inst)
        _slots.push_back(slot2);

    SlotArtifact slot3 = createSlotArtifact("slotArtifact3", "reset3", "portalOcean8_Ocean9");
    if(slot3.inst)
        _slots.push_back(slot3);

    SlotArtifact slot4 = createSlotArtifact("slotArtifact4", "reset4", "");
    if(slot4.inst)
        _slots.push_back(slot4);

    SlotArtifact slot5 = createSlotArtifact("slotArtifact5", "reset5", "");
    if(slot5.inst)
        _slots.push_back(slot5);

    for(size_t i=0 ; i<level().objects.size() ; ++i)
    {
        if(level().physObjects[i] && level().objects[i].model == "caisse")
        {
            bindSound(level().physObjects[i], PortalGame::SoundEffects::WOOD3);
        }
        else if(level().physObjects[i] && level().objects[i].name == "oceanArtifact")
        {
            bindSound(level().physObjects[i], PortalGame::SoundEffects::ARTIFACT1);
        }
        else if(level().physObjects[i] && level().objects[i].model == "palmLog")
        {
            bindSound(level().physObjects[i], PortalGame::SoundEffects::WOOD1);
        }
    }
}

void OceanLevel::prepareEnter()
{
    _ambientOceanSource->play();
}

void OceanLevel::beforeLeave()
{
    _ambientOceanSource->stop();
}

#define TIME_ON_SLOT 0.5f

void OceanLevel::update(float time)
{
    vec3 artifactPos = level().objects[_artifactIndex].meshInstance->matrix().translation();
    bool onSlot = false;

    for(SlotArtifact& slot : _slots)
    {
        if((slot.inst->matrix().translation() - artifactPos).length() < 0.02)
        {
            onSlot = true;
            slot.timeOn += time;
            if(slot.timeOn > TIME_ON_SLOT && slot.resetActive == false)
            {
                slot.resetActive = true;
                if(slot.resetButtonIndex >= 0)
                {
                    interface::Mesh m = level().objects[slot.resetButtonIndex].meshInstance->mesh();
                    m.element(0).setEmissive(0.7);
                    level().objects[slot.resetButtonIndex].meshInstance->setMesh(m);
                }

                if(slot.portal)
                    setEnablePortal(true, slot.portal);

                Source* src = levelSystem().listener().addSource(_warpSound);
                src->setPosition(slot.inst->matrix().translation());
                src->play();
                src->release();
            }
        }
        else
        {
            slot.timeOn = 0;
        }

        if(slot.resetActive && slot.resetButtonIndex >= 0)
        {
            if(collidePaddles(level().physObjects[slot.resetButtonIndex]))
            {
                if(!slot.onReset)
                {
                    slot.onReset = true;
                    BulletObject* objArt = level().physObjects[_artifactIndex];

                    objArt->body()->setLinearVelocity(btVector3(0,0,0));
                    objArt->body()->setAngularVelocity(btVector3(0,0,0));
                    objArt->body()->setWorldTransform(BulletObject::toBtTransform(slot.inst->matrix()));

                    if(_timerButtonSound > 0.3)
                    {
                        Source* src = levelSystem().listener().addSource(_buttonSound);
                        src->setPosition(level().objects[slot.resetButtonIndex].meshInstance->matrix().translation());
                        src->play();
                        src->release();
                        _timerButtonSound = 0;
                    }
                }
            }
            else slot.onReset = false;
        }
    }

    _timerButtonSound += time;

    if(onSlot)
        _timeOnSlot += time;
    else
        _timeOnSlot -= time;

    _timeOnSlot = std::min(std::max(_timeOnSlot, 0.f), TIME_ON_SLOT);
    interface::Mesh m = level().objects[_artifactIndex].meshInstance->mesh();
    for(size_t i=0 ; i<m.nbElements() ; ++i)
        m.element(i).setEmissive(0.9f * _timeOnSlot / TIME_ON_SLOT);

    level().objects[_artifactIndex].meshInstance->setMesh(m);
}

OceanLevel::SlotArtifact OceanLevel::createSlotArtifact(std::string nameSlot, std::string resetButton, std::string portal, bool alreadyActive)
{
    SlotArtifact slot;
    int index = indexObject(nameSlot);
    if(index >= 0)
        slot.inst = level().objects[index].meshInstance;
     else
        slot.inst = nullptr;

    slot.resetButtonIndex = indexObject(resetButton);
    slot.resetActive = alreadyActive;

    if(!portal.empty())
    {
        index = indexObject(portal);
        if(index >= 0)
        {
            slot.portal = level().objects[index].meshInstance;
            setEnablePortal(false, slot.portal);
        }
         else
            slot.portal = nullptr;
    }
    else
        slot.portal = nullptr;

    return slot;
}

 void OceanLevel::callDebug()
 {
     BulletObject* objArt = level().physObjects[_artifactIndex];
     objArt->body()->setLinearVelocity(btVector3(0,0,0));
     objArt->body()->setAngularVelocity(btVector3(0,0,0));
     objArt->body()->setWorldTransform(BulletObject::toBtTransform(
                     mat4::Translation(levelSystem().controller().controllerInfo().leftHand->matrix().translation() + vec3(0,0,-0.2))));
 }
