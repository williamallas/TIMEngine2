#include "SacredGroveLevel.h"
#include "resource/AssetManager.h"
#include "bullet/BulletObject.h"
#include "openAL/Source.hpp"
#include "PortalGame.h"
#include "Rand.h"

#include "MemoryLoggerOn.h"


SacredGroveBase::SacredGroveBase(int index, LevelSystem* system, BulletEngine& phys) : LevelInterface(index, system), _physEngine(phys)
{
    resource::SoundAsset ambientSound = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/sacred_grove.ogg", true, Sampler::NONE).value();
    Source* src = system->listener().addSource(ambientSound);
    src->setLooping(true);
    src->setGain(0.2);
    setAmbientSound(src, "sacred_grove");
}

/** SacredGroveAux **/

SacredGroveAux::SacredGroveAux(int index, LevelSystem* system, BulletEngine& phys, std::string color) : SacredGroveBase(index, system, phys), _color(color)
{

}

void SacredGroveAux::init()
{
    _indexSlot = indexObject("column");
    _indexArtifact = indexObject("sg_artifact_" + _color);

    if(_indexArtifact >= 0)
        registerGameObject(_indexArtifact, std::string("sg_artifact_") + _color);
}

void SacredGroveAux::update(float time)
{
    if(!level().physObjects[_indexArtifact])
        return;

    if(level().physObjects[_indexArtifact]->indexWorld() == index() && _indexSlot >= 0)
    {
        vec3 tr = level().objects[_indexArtifact].meshInstance->matrix().translation();
        vec3 slot_tr = level().objects[_indexSlot].meshInstance->matrix().translation();
        if((tr - slot_tr - vec3(0,0,1.372)).length() < 0.1)
        {
            interface::Mesh m = level().objects[_indexArtifact].meshInstance->mesh();
            m.element(2).setEmissive(std::min(0.25f, m.element(2).emissive() + time*0.2f));
            level().objects[_indexArtifact].meshInstance->setMesh(m);
            _isArtifactBright = true;
        }
        else
        {
            interface::Mesh m = level().objects[_indexArtifact].meshInstance->mesh();
            m.element(2).setEmissive(std::max(0.04f, m.element(2).emissive() - time*0.2f));
            level().objects[_indexArtifact].meshInstance->setMesh(m);
            _isArtifactBright = false;
        }
    }
}

/** SacredGroveMain **/

SacredGroveMain::SacredGroveMain(int index, LevelSystem* system, BulletEngine& phys) : SacredGroveBase(index, system, phys)
{

}

void SacredGroveMain::init()
{
    _indexButtons[0] = indexObject("buttonRed");
    _indexButtons[1] = indexObject("buttonWhite");
    _indexButtons[2] = indexObject("buttonBlue");

    _indexPortals[0] = indexObject("portalGroveIn_Forest3Out");
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


}

void SacredGroveMain::update(float time)
{
    for(int i=0 ; i<3 ; ++i)
    {
        if(_activeButton != i && _buttonObj[i] &&
           (levelSystem().controller().controllerInfo().leftHandPhys->collideWith(_buttonObj[i]->body()).size() > 0 ||
            levelSystem().controller().controllerInfo().rightHandPhys->collideWith(_buttonObj[i]->body()).size() > 0))
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
            break;
        }

    }

}
