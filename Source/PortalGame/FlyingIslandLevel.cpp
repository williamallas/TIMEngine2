#include "FlyingIslandLevel.h"

#include "PortalGame.h"
#include "Rand.h"
#include "SimpleSpecProbeImportExport.h"
#include "resource/AssetManager.h"
#include <thread>

FlyingIslandLevel::FlyingIslandLevel(int index, LevelSystem* system, BulletEngine& phys, Sync_Ocean_FlyingIsland_PTR syncObj)
    : LevelInterface(index, system), _physEngine(phys), _syncBoat(syncObj)
{
    _buttonSound = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/pheub.wav", false, Sampler::NONE).value();
    _warp = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/warp.wav", false, Sampler::NONE).value();
    _connectFourSound = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/plastic2.wav", false, Sampler::NONE).value();

    _ringShape = new btSphereShape(0.325*1.6);

    resource::SoundAsset ambientSound = resource::AssetManager<resource::SoundAsset>::instance().load<false>("soundBank/bensound-betterdays.ogg", true, Sampler::NONE).value();
    Source* src = system->listener().addSource(ambientSound);
    src->setLooping(true);
    src->setGain(0.12);
    setAmbientSound(src, "bensound-betterdays");
}

FlyingIslandLevel::~FlyingIslandLevel()
{
    delete _ringShape;
}

void FlyingIslandLevel::init()
{
    for(size_t i=0 ; i<level().objects.size() ; ++i)
    {
        if(level().physObjects[i] && level().objects[i].name == "cube1")
            bindSound(level().physObjects[i], PortalGame::SoundEffects::WOOD2);
        else if(level().physObjects[i] && level().objects[i].name == "cube2")
            bindSound(level().physObjects[i], PortalGame::SoundEffects::SOUR);
        else if(level().physObjects[i] && level().objects[i].name == "cube3")
            bindSound(level().physObjects[i], PortalGame::SoundEffects::ROCK1);
    }

    _physEngine.setGravity(level().indexScene, vec3(0,0,0));

    auto lpVec = LightProbeUtils::importProbe("flyingIsland_specprobe.xml");
    for(auto lp : lpVec)
    {
        auto l = LightProbeUtils::genLightProbe(lp);
        if(l.tex)
            level().levelScene->scene.add<interface::LightInstance>(l);
    }

    {
        int indexBoat = indexObject("boat");
        if(indexBoat >= 0)
            _syncBoat->boatFI = level().objects[indexBoat].meshInstance;
    }

    for(int i=0 ; i<2 ; ++i)
    {
        Elevator elev;
        elev.elevatorObject = indexObject("elevator" + StringUtils(i+1).str());
        elev.elevatorObjectArrival = indexObject("elevator" + StringUtils(i+1).str() + "_arrival");
        elev.indexButton[0] = indexObject("button" + StringUtils(i+1).str() + "_1");
        elev.indexButton[1] = indexObject("button" + StringUtils(i+1).str() + "_2");
        if(elev.elevatorObject >= 0)
            _elevators.push_back(elev);
    }

    {
        Elevator elev;
        elev.elevatorObject = indexObject("elevator2");
        elev.elevatorObjectArrival = indexObject("elevator2_arrival2");
        elev.indexButton[0] = indexObject("button3_1");
        elev.indexButton[1] = indexObject("button3_2");
        elev.needToWin = true;
        if(elev.elevatorObject >= 0)
            _elevators.push_back(elev);
    }

    auto cubes1 = indexObjects("cube1");
    auto cubes2 = indexObjects("cube2");
    auto cubes3 = indexObjects("cube3");
    cubes1.insert(cubes1.end(), cubes2.begin(), cubes2.end());
    cubes1.insert(cubes1.end(), cubes3.begin(), cubes3.end());
    for(int index : cubes1)
    {
        if(level().physObjects[index])
        {
            level().physObjects[index]->body()->setGravity(btVector3(0,0,0));
            _cubes.push_back(index);
            registerPortableTraversable(index, level().objects[index].meshInstance, level().physObjects[index], {});
        }
    }

    for(int index : indexObjects("ring"))
    {
        auto ptr = std::unique_ptr<BulletObject>(new BulletObject(mat4::Translation(level().objects[index].translation), _ringShape, 0));
        _rings.push_back({index, false, std::move(ptr)});
    }

    _portal1Index = indexObject("portalFI1_FI2");
    if(_portal1Index >= 0)
        setEnablePortal(false, level().objects[_portal1Index].meshInstance);

    /* init connectFour game */
    int gridGameIndex = indexObject("connectFour");
    if(gridGameIndex >= 0)
    {
        _4game.transformation = level().objects[gridGameIndex].meshInstance->matrix();
        _4game.scale = level().objects[gridGameIndex].scale;

        for(uint i=0 ; i<ConnectFourIA::GRID_X ; ++i)
            for(uint j=0 ; j<ConnectFourIA::GRID_Y ; ++j)
                _4game.toGo[i][j] = _4game.getTransFromCoord({i,j}).translation();

        if(indexObject("redCoin"))
            _4game.coins[0] = level().objects[indexObject("redCoin")].meshInstance->mesh();
        if(indexObject("yellowCoin"))
            _4game.coins[1] = level().objects[indexObject("yellowCoin")].meshInstance->mesh();

    }


//    level().levelScene->scene.add<interface::MeshInstance>(level().objects[indexObject("yellowCoin")].meshInstance->mesh(),
//            _4game.getTransFromCoord({0,1}));

//    level().levelScene->scene.add<interface::MeshInstance>(level().objects[indexObject("redCoin")].meshInstance->mesh(),
//            _4game.getTransFromCoord({0,2}));

    for(int i=0 ; i<7 ; ++i)
        _indexButtons[i] = indexObject("cfButton" + StringUtils(i+1).str());
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

            if((_hasWin || !elev.needToWin) && elev.activateTime[i] <= 0 && level().physObjects[elev.indexButton[i]] && collidePaddles(level().physObjects[elev.indexButton[i]]))
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


        if(launch && elev.state != 1) // active the elevator
        {
            elev.state = 1;
            level().objects[elev.elevatorObject].translation = level().objects[elev.elevatorObject].meshInstance->matrix().translation();
        }

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

    /* update cubes */
    int numberCloseCube = 0;
    btVector3 gravityCenter = btVector3(41, -45, 17.75);
    vector<int> newCubes;
    newCubes.reserve(_cubes.size());

//    if((levelSystem().headPosition() - vec3(41, -45, 17.75)).length2() > 8*8)
//        return;

    for(int index : _cubes)
    {
        if(level().physObjects[index]->indexWorld() != level().indexScene)
            continue;

        testCubeOnRing(level().physObjects[index]);

        newCubes.push_back(index);
        auto body = level().physObjects[index]->body();  
        body->activate();

        btVector3 objToArena = btVector3(gravityCenter - body->getWorldTransform().getOrigin());
        if(objToArena.length2() < 1.6*1.6)
        {
            numberCloseCube++;
            continue;
        }

        body->applyCentralForce(-body->getLinearVelocity()*0.05);
        body->applyTorque(-body->getAngularVelocity()*0.01);

        if(objToArena.length2() > 30*30)
            continue;

        if(body->getLinearVelocity().length2() < 0.1*0.1 && _nbCloseCube < 8)
            body->applyCentralForce(btVector3(gravityCenter - body->getWorldTransform().getOrigin()).normalized()*0.2);
    }

    _nbCloseCube = numberCloseCube;
    _cubes = newCubes;

    _4game.update(time);
    _delayBeforeNextPlay -= time;

    if(_delayResetGame > 0)
    {
        _delayResetGame -= time;
        if(_delayResetGame <= 0)
        {
            _4game.reset(level().levelScene->scene);
            _delayBeforeNextPlay = 0;
        }
    }

    if(!_4game.ia.humanTurn())
        return;

   if(!_firstTurn && _futureMove.valid() && _futureMove.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
   {
       int res = _futureMove.get();
       _futureMove = std::future<int>();

       if(res >= 0)
       {
            _4game.jetons[res][_4game.ia.getVerticalIndex(res)-1] =
                    &level().levelScene->scene.add<interface::MeshInstance>(_4game.coins[1], _4game.getTransFromCoord({(uint)res, ConnectFourIA::GRID_Y-1}));

            emitSound(level().objects[_indexButtons[res]].translation, _connectFourSound);

            if(_4game.ia.checkWinner() == ConnectFourIA::P2)
                _delayResetGame = 3;
       }

       resetButtons();
   }

   if(_4game.ia.checkWinner() != ConnectFourIA::Slot::EMPTY)
       return;

    if(_delayBeforeNextPlay > 0)
        return;

    for(int i=0 ; i<7 ; ++i)
    {
        if(_indexButtons[i] < 0)
            continue;


        if(level().physObjects[_indexButtons[i]] && collidePaddles(level().physObjects[_indexButtons[i]]))
        {
            if(_4game.ia.humanPlay(i))
            {
                //std::cout << _4game.ia.getVerticalIndex(i)-1 << " i : " << i << std::endl;
                interface::Mesh m = level().objects[_indexButtons[i]].meshInstance->mesh();
                m.element(0).setEmissive(0.2);
                level().objects[_indexButtons[i]].meshInstance->setMesh(m);

                _4game.jetons[i][_4game.ia.getVerticalIndex(i)-1] =
                        &level().levelScene->scene.add<interface::MeshInstance>(_4game.coins[0], _4game.getTransFromCoord({(uint)i, ConnectFourIA::GRID_Y-1}));

                if(_4game.ia.checkWinner() != ConnectFourIA::Slot::EMPTY)
                {
                    _hasWin = true;
                    break;
                }

                _futureMove = std::move(renderer::globalThreadPool.schedule_trace([&](){ std::this_thread::sleep_for(std::chrono::seconds(1)); return _4game.ia.computerPlay(); }));
                _firstTurn = false;
                _delayBeforeNextPlay = DELAY_PLAY;

                emitSound(level().objects[_indexButtons[i]].translation, _connectFourSound);
            }
            break;
        }
    }
}

void FlyingIslandLevel::resetButtons()
{
    for(int i=0 ; i<7 ; ++i)
    {
        if(_indexButtons[i] < 0)
            continue;

        interface::Mesh m = level().objects[_indexButtons[i]].meshInstance->mesh();
        m.element(0).setEmissive(0);
        level().objects[_indexButtons[i]].meshInstance->setMesh(m);
    }
}

void FlyingIslandLevel::prepareEnter()
{

}

void FlyingIslandLevel::beforeLeave()
{

}

void FlyingIslandLevel::testCubeOnRing(BulletObject* bObj)
{
    if(_ringsEnabled)
        return;

    bool allEnable = true;
    for(Ring& ring : _rings)
    {
    #ifdef AUTO_SOLVE
        ring.enabled = true;
    #endif
        if(!ring.enabled && bObj->collideWith(ring.collObj->body()).size() > 0)
        {
            ring.enabled = true;
            interface::Mesh m = level().objects[ring.indexObject].meshInstance->mesh();
            if(m.nbElements() > 0)
                m.element(0).setColor(vec4(221/255.f, 1, 100/255.f, 1));
            level().objects[ring.indexObject].meshInstance->setMesh(m);
        }
        allEnable = allEnable && ring.enabled;
    }

    _ringsEnabled = allEnable;
    if(allEnable && _portal1Index >= 0)
    {
        setEnablePortal(true, level().objects[_portal1Index].meshInstance);
        emitSound(level().objects[_portal1Index].translation + vec3(0,0,1), _warp);
    }

}

void FlyingIslandLevel::ConnectFourGame::reset(interface::SimpleScene& sc)
{
    for(int i=0 ; i<ConnectFourIA::GRID_X ; ++i)
        for(int j=0 ; j<ConnectFourIA::GRID_Y ; ++j)
        {
            if(jetons[i][j])
                sc.remove(*jetons[i][j]);

            jetons[i][j] = nullptr;
        }

    ia = ConnectFourIA();
}

void FlyingIslandLevel::ConnectFourGame::update(float time)
{
    for(int i=0 ; i<ConnectFourIA::GRID_X ; ++i)
        for(int j=0 ; j<ConnectFourIA::GRID_Y ; ++j)
        {
            if(!jetons[i][j]) continue;

            mat4 m = jetons[i][j]->matrix();
            if(m.translation().z() > toGo[i][j].z())
                m.setTranslation(m.translation() - vec3(0,0,time*0.5));

            jetons[i][j]->setMatrix(m);
        }
}

mat4 FlyingIslandLevel::ConnectFourGame::getTransFromCoord(uivec2 coord) const
{
    vec3 origin(0, -0.859, 0.144);
    vec3 corner(0, 0.858, 1.57496);
    float z_offset = 0.146;

    vec3 step = (corner-origin) / vec3(1, ConnectFourIA::GRID_X-1, ConnectFourIA::GRID_Y-1);
    step.y() = -step.y();

    return transformation * mat4::Translation(step*vec3(0,coord.x(), coord.y()) - vec3(0,origin.y()-corner.y(),0)*0.5 + vec3(0,0,z_offset));
}

