#include "FlyingIslandLevel.h"

#include "PortalGame.h"
#include "Rand.h"
#include "SimpleSpecProbeImportExport.h"
#include "resource/AssetManager.h"

FlyingIslandLevel::FlyingIslandLevel(int index, LevelSystem* system, BulletEngine& phys, Sync_Ocean_FlyingIsland_PTR syncObj)
    : LevelInterface(index, system), _physEngine(phys), _syncBoat(syncObj)
{
    _buttonSound = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/pheub.wav", false, Sampler::NONE).value();

}

FlyingIslandLevel::~FlyingIslandLevel()
{

}

void FlyingIslandLevel::init()
{
    auto lpVec = LightProbeUtils::importProbe("flyingIsland_specprobe.xml");
    for(auto lp : lpVec)
    {
        level().levelScene->scene.add<interface::LightInstance>(LightProbeUtils::genLightProbe(lp));
    }

    {
        int indexBoat = indexObject("boat");
        if(indexBoat >= 0)
            _syncBoat->boatFI = level().objects[indexBoat].meshInstance;
    }

    for(int i=0 ; i<1 ; ++i)
    {
        Elevator elev;
        elev.elevatorObject = indexObject("elevator" + StringUtils(i+1).str());
        elev.elevatorObjectArrival = indexObject("elevator" + StringUtils(i+1).str() + "_arrival");
        elev.indexButton[0] = indexObject("button" + StringUtils(i+1).str() + "_1");
        elev.indexButton[1] = indexObject("button" + StringUtils(i+1).str() + "_2");
        if(elev.elevatorObject >= 0)
            _elevators.push_back(elev);
    }

//    if(!_fillSkyAssets.empty())
//    {
//        Rand rand(12);
//        const auto SIZE_WORLD = vec3(100,100,80);
//        const auto CENTER_WORLD = vec3(45, -45, 0);
//        for(int i=0 ; i<800 ; ++i)
//        {
//            vec3 v = vec3(rand.next_f(),rand.next_f(),rand.next_f()) * SIZE_WORLD - SIZE_WORLD*0.5;
//            mat4 m = mat4::RotationZ(toRad(rand.next_f()*360));
//            m.setTranslation(v + CENTER_WORLD);

//            int r = rand.next_i()%100;
//            if(r < 40)      level().levelScene->scene.add<interface::MeshInstance>(_fillSkyAssets[0], m);
//            else if(r < 90) level().levelScene->scene.add<interface::MeshInstance>(_fillSkyAssets[1], m);
//            else if(r < 95) level().levelScene->scene.add<interface::MeshInstance>(_fillSkyAssets[2], m);
//            else            level().levelScene->scene.add<interface::MeshInstance>(_fillSkyAssets[3], m);
//        }
//    }
}

void FlyingIslandLevel::update(float time)
{
    if(_enterWithBoat && _syncBoat->boatOcean)
    {
        float step = time;
        if(_syncBoat->remainingDist - time <= 0)
        {
            step = _syncBoat->remainingDist;
            _enterWithBoat = false;
        }

        mat4 m = _syncBoat->boatOcean->matrix();
        m.translate(_syncBoat->dir * step);
        levelSystem().hmdView().addOffset(mat4::Translation(_syncBoat->dir * step));

        _syncBoat->boatOcean->setMatrix(m);
        _syncBoat->boatFI->setMatrix(m);
        _syncBoat->remainingDist -= time;
    }

    for(Elevator& elev : _elevators)
    {
        bool launch = true;
        for(int i=0 ; i<2 ; ++i)
        {
            interface::Mesh m = level().objects[elev.indexButton[i]].meshInstance->mesh();

            if(elev.activateTime[i] <= 0 && level().physObjects[elev.indexButton[i]] && collidePaddles(level().physObjects[elev.indexButton[i]]))
            {
                Source* src = levelSystem().listener().addSource(_buttonSound);
                src->setPosition( level().objects[elev.indexButton[i]].translation );
                src->play();
                src->release();
                elev.activateTime[i] = 5;
                m.element(0).setEmissive(0.5);
            }
            else if(elev.activateTime[i] <= 0)
            {
                m.element(0).setEmissive(0);
                launch = false;
            }
            else
            {
                elev.activateTime[i] -= time;
            }

            level().objects[elev.indexButton[i]].meshInstance->setMesh(m);
        }


        if(launch) // active the elevator
            elev.state = 1;

        if(elev.state == 1)
        {
            vec3 dir = level().objects[elev.elevatorObjectArrival].translation - level().objects[elev.elevatorObject].translation;
            float distToMove = time;
            float targetDist = dir.length();
            dir /= targetDist;

            if(elev.distance + distToMove > targetDist)
            {
                distToMove = targetDist - elev.distance;
                elev.state = 2;
            }
            elev.distance += time;
            mat4 m = level().objects[elev.elevatorObject].meshInstance->matrix();
            m.translate(dir * distToMove);
            level().objects[elev.elevatorObject].meshInstance->setMatrix(m);
            levelSystem().hmdView().addOffset(mat4::Translation(dir * distToMove));
        }
    }
}

void FlyingIslandLevel::prepareEnter()
{

}

void FlyingIslandLevel::beforeLeave()
{

}

