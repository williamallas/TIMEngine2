#include "StartLevel.h"
#include "SimpleSpecProbeImportExport.h"
#include "PortalGame.h"

StartLevel::StartLevel(int index, LevelSystem* system) : LevelInterface(index, system)
{

}

void StartLevel::init()
{
    auto lpVec = LightProbeUtils::importProbe("start_specprobe.xml");
    for(auto lp : lpVec)
    {
        level().levelScene->scene.add<interface::LightInstance>(LightProbeUtils::genLightProbe(lp));
    }

    for(size_t i=0 ; i < level().objects.size() ; ++i)
    {
        LevelSystem::GraphicGameObject& obj = level().objects[i];
        if(obj.model == "box" && !obj.isStatic && level().physObjects[i] && obj.scale.x() > 0.15)
        {
            bindSound(level().physObjects[i], PortalGame::SoundEffects::PLASTIC2);
            registerPortableTraversable(-1, obj.meshInstance, level().physObjects[i], {"portalBegin_ForestIn"});
        }
        if(obj.model == "box" && !obj.isStatic && level().physObjects[i] && obj.scale.x() < 0.15)
        {
            bindSound(level().physObjects[i], PortalGame::SoundEffects::PLASTIC1);
            registerPortableTraversable(-1, obj.meshInstance, level().physObjects[i], {"portalBegin_ForestIn"});
        }

        else if(obj.name == "goldSphere" && !obj.isStatic && level().physObjects[i])
        {
            bindSound(level().physObjects[i], PortalGame::SoundEffects::METAL1);
            registerPortableTraversable(-1, obj.meshInstance, level().physObjects[i], {"portalBegin_ForestIn"});
        }
    }

    _indexPortal = indexObject("portalStart_Start2In");
    _indexStack[0] = indexObject("1b");
    _indexStack[1] = indexObject("2b");
    _indexStack[2] = indexObject("3b");
    _indexStack[3] = indexObject("4s");
    _indexStack[4] = indexObject("5s");

    _indexBalls = indexObjects("goldSphere");

    if(_indexPortal >= 0)
        setEnablePortal(false, level().objects[_indexPortal].meshInstance);
}

void StartLevel::prepareEnter()
{

}

void StartLevel::update(float time)
{
    vec2 center = level().objects[_indexStack[0]].meshInstance->matrix().translation().to<2>();

    float heightCubes[5] = { 0.2, 0.6, 1, 1.324, 1.571 };

    bool cubeAligned = true;
    bool correctHeight = true;

    for(int i=0 ; i<5 ; ++i)
    {
        vec3 pos = level().objects[_indexStack[i]].meshInstance->matrix().translation();
        if((pos.to<2>() - center).length2() > 0.5*0.5)
        {
            cubeAligned = false;
            break;
        }

        if(fabsf(pos.z() - heightCubes[i]) > 0.1)
        {
            correctHeight = false;
            break;
        }
    }

    bool ballsInPlace[4] = {false};
    for(int index : _indexBalls)
    {
        if((level().objects[index].meshInstance->matrix().translation() - vec3(1.2,1.2,1.2)).length2() < 0.2*0.2)
            ballsInPlace[0] = true;
        else if((level().objects[index].meshInstance->matrix().translation() - vec3(-1.2 ,1.2, 1.2)).length2() < 0.2*0.2)
            ballsInPlace[1] = true;
        else if((level().objects[index].meshInstance->matrix().translation() - vec3(1.2, -1.2, 1.2)).length2() < 0.2*0.2)
            ballsInPlace[2] = true;
        else if((level().objects[index].meshInstance->matrix().translation() - vec3(-1.2, -1.2, 1.2)).length2() < 0.2*0.2)
            ballsInPlace[3] = true;
    }

    if(cubeAligned && correctHeight && ballsInPlace[0] && ballsInPlace[1] && ballsInPlace[2] && ballsInPlace[3])
        _timeSinceOk += time;
    else
        _timeSinceOk = 0;

#ifndef AUTO_SOLVE
    if(_timeSinceOk > 1)
#endif
        setEnablePortal(true, level().objects[_indexPortal].meshInstance);
}


Start2Level::Start2Level(int index, LevelSystem* system) : LevelInterface(index, system)
{

}

void Start2Level::init()
{
    for(size_t i=0 ; i < level().objects.size() ; ++i)
    {
        LevelSystem::GraphicGameObject& obj = level().objects[i];
        if(obj.name == "tCube" && level().physObjects[i])
        {
            bindSound(level().physObjects[i], PortalGame::SoundEffects::PLASTIC2);
            registerPortableTraversable(-1, obj.meshInstance, level().physObjects[i], {"portalBegin_ForestIn"});
        }
        if(obj.name == "smallCube" && level().physObjects[i])
        {
            bindSound(level().physObjects[i], PortalGame::SoundEffects::PLASTIC1);
            //registerPortableTraversable(-1, obj.meshInstance, level().physObjects[i], {"portalBegin_ForestIn"});
        }

        else if(obj.name == "spaceCube" && level().physObjects[i])
        {
            bindSound(level().physObjects[i], PortalGame::SoundEffects::METAL1);
            registerPortableTraversable(-1, obj.meshInstance, level().physObjects[i], {});
        }
    }
}

void Start2Level::prepareEnter()
{

}

void Start2Level::update(float)
{

}

