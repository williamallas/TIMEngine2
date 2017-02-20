#include "SacredGroveLevel.h"
#include "resource/AssetManager.h"
#include "bullet/BulletObject.h"
#include "openAL/Source.hpp"
#include "PortalGame.h"
#include "CollisionMask.h"
#include "Rand.h"

#include "MemoryLoggerOn.h"


SacredGroveBase::SacredGroveBase(int index, LevelSystem* system, BulletEngine& phys) : LevelInterface(index, system), _physEngine(phys)
{
    resource::SoundAsset ambientSound = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/sacred_grove.ogg", true, Sampler::NONE).value();
    Source* src = system->listener().addSource(ambientSound);
    src->setLooping(true);
    src->setGain(0.15);
    setAmbientSound(src, "sacred_grove");
}

bool SacredGroveBase::updateArtifact(const LevelSystem::GameObject& obj, int indexSlot, float height, float time)
{
    if(obj.sceneIndex == index() && indexSlot >= 0)
    {
        vec3 tr = obj.object->meshInstance->matrix().translation();
        vec3 slot_tr = level().objects[indexSlot].meshInstance->matrix().translation();
        if((tr - slot_tr - vec3(0,0,height)).length() < 0.5)
        {
            interface::Mesh m = obj.object->meshInstance->mesh();
            m.element(2).setEmissive(std::min(0.25f, m.element(2).emissive() + time*0.2f));
            obj.object->meshInstance->setMesh(m);
            return true;
        }
        else
        {
            interface::Mesh m = obj.object->meshInstance->mesh();
            m.element(2).setEmissive(std::max(0.04f, m.element(2).emissive() - time*0.2f));
            obj.object->meshInstance->setMesh(m);
            return false;
        }
    }
    return false;
}

/** SacredGroveAux **/

SacredGroveAux::SacredGroveAux(int index, LevelSystem* system, BulletEngine& phys, std::string color) : SacredGroveBase(index, system, phys), _color(color)
{
    _indexSlot = indexObject("column");
    _indexArtifact = indexObject("sg_artifact_" + _color);

    if(_indexArtifact >= 0)
    {
        level().physObjects[_indexArtifact]->setMask(CollisionTypes::COL_IOBJ, IOBJECT_COLLISION);
        registerGameObject(_indexArtifact, std::string("sg_artifact_") + _color);
    }
}

void SacredGroveAux::init()
{
    if(_indexArtifact >= 0)
    {
        registerPortableTraversable(_indexArtifact, level().objects[_indexArtifact].meshInstance, level().physObjects[_indexArtifact], {});
        bindSound(level().physObjects[_indexArtifact], PortalGame::METAL1);
    }
}

void SacredGroveAux::update(float time)
{  
    if(_indexArtifact >= 0)
    {
        if(!level().physObjects[_indexArtifact])
            return;

        LevelSystem::GameObject obj;
        obj.sceneIndex = level().physObjects[_indexArtifact]->indexWorld();
        obj.object = &level().objects[_indexArtifact];
        obj.physObject = level().physObjects[_indexArtifact];

        _isArtifactBright =  updateArtifact(obj, _indexSlot, 1.372, time);
    }
}

/** SacredGroveMain **/

SacredGroveMain::SacredGroveMain(int index, LevelSystem* system, BulletEngine& phys) : SacredGroveBase(index, system, phys)
{
    _buttonSound = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/pheub.wav", false, Sampler::NONE).value();
    _warp = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/warp.wav", false, Sampler::NONE).value();
}

void SacredGroveMain::init()
{
    _indexButtons[0] = indexObject("buttonRed");
    _indexButtons[1] = indexObject("buttonWhite");
    _indexButtons[2] = indexObject("buttonBlue");

    _indexColumns[0] = indexObject("column_red");
    _indexColumns[1] = indexObject("column_white");
    _indexColumns[2] = indexObject("column_blue");

    _indexPortals[0] = indexObject("portalGroveIn_Forest3Out");
    _indexPortals[1] = indexObject("portalGroveOut_OceanIn");
    _indexPortals[2] = indexObject("portalGroveToRed_GroveRed");
    _indexPortals[3] = indexObject("portalGroveToWhite_GroveWhite");
    _indexPortals[4] = indexObject("portalGroveToBlue_GroveBlue");

    for(int i=0 ; i<3 ; ++i)
    {
        _buttonObj[i] = level().physObjects[_indexButtons[i]];
        _buttonInst[i] = level().objects[_indexButtons[i]].meshInstance;
        if(_indexPortals[i+2] >= 0)
            setEnablePortal(false, level().objects[_indexPortals[i+2]].meshInstance);
    }

    if(_indexPortals[1] >= 0)
        setEnablePortal(false, level().objects[_indexPortals[1]].meshInstance);
}

void SacredGroveMain::update(float time)
{
    if(!_allActive)
    {
        for(int i=0 ; i<3 ; ++i)
        {
            if(_activeButton != i && _buttonObj[i] && collidePaddles(_buttonObj[i]))
            {
                if(_activeButton >= 0)
                {
                    interface::Mesh m = _buttonInst[_activeButton]->mesh();
                    m.element(0).setEmissive(0);
                    _buttonInst[_activeButton]->setMesh(m);

                    if(_indexPortals[_activeButton+2] >= 0)
                        setEnablePortal(false, level().objects[_indexPortals[_activeButton+2]].meshInstance);
                }

                interface::Mesh m = _buttonInst[i]->mesh();
                m.element(0).setEmissive(0.5);
                _buttonInst[i]->setMesh(m);
                _activeButton = i;

                if(_indexPortals[i+2] >= 0)
                {
                    setEnablePortal(false, level().objects[_indexPortals[0]].meshInstance);
                    setEnablePortal(true, level().objects[_indexPortals[i+2]].meshInstance);
                }

                Source* src = levelSystem().listener().addSource(_buttonSound);
                src->setPosition({-0.898, 0.436, 1.508});
                src->play();
                src->release();

                break;
            }
        }
    }

    Option<LevelSystem::GameObject> artifacts[3];
    artifacts[0] = getGameObject("sg_artifact_red");
    artifacts[1] = getGameObject("sg_artifact_white");
    artifacts[2] = getGameObject("sg_artifact_blue");

    bool allActive = true;
    for(int i=0 ; i<3 ; ++i)
    {
        bool b = false;
        if(artifacts[i].hasValue())
        {
            b = updateArtifact(artifacts[i].value(), _indexColumns[i], 1.372, time);

            if(_artifactOnSlot[i] != b)
                _artifactOnSlot[i] = b;
        }
        allActive = allActive && b;
    }

    if(allActive && !_allActive)
    {
        if(_indexPortals[0] >= 0)
            setEnablePortal(false, level().objects[_indexPortals[0]].meshInstance);

        for(int i=0 ; i<3 ; ++i)
        {
            if(_indexPortals[i+2] >= 0)
            {
                setEnablePortal(false, level().objects[_indexPortals[i+2]].meshInstance);
                interface::Mesh m = _buttonInst[i]->mesh();
                m.element(0).setEmissive(0);
                _buttonInst[i]->setMesh(m);
            }
        }

        if(_indexPortals[1] >= 0)
            setEnablePortal(true, level().objects[_indexPortals[1]].meshInstance);

        Source* src = levelSystem().listener().addSource(_warp);
        src->setPosition({-0.898, 0.436, 1.508});
        src->play();
        src->release();
    }
    else if(_allActive && !allActive)
    {
        if(_indexPortals[1] >= 0)
            setEnablePortal(false, level().objects[_indexPortals[1]].meshInstance);
    }

#ifdef AUTO_SOLVE
    for(int i=0 ; i<5 ; ++i)
    {
        if(_indexPortals[i] >= 0)
            setEnablePortal(false, level().objects[_indexPortals[i]].meshInstance);
    }
    if(_indexPortals[1] >= 0)
        setEnablePortal(true, level().objects[_indexPortals[1]].meshInstance);
#endif

    _allActive = allActive;
}
