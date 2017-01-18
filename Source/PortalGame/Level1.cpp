#include "Level1.h"
#include "interface/ShaderPool.h"
#include "SimpleSpecProbeImportExport.h"

Level1::Level1(int index, LevelSystem* system) : LevelInterface(index, system)
{

}

void Level1::init()
{
//    _indexPortal = indexObject("portalBegin_ForestIn");
//    if(_indexPortal < 0)
//        LOG("portalBegin_ForestIn in scene1 not found");
//    else
//        setEnablePortal(false, level().objects[_indexPortal].meshInstance);

    LevelSystem::Level& l = level();
    for(size_t i=0 ; i<l.objects.size() ; ++i)
    {
        if(l.physObjects[i])
            registerPortableTraversable(-1, l.objects[i].meshInstance, l.physObjects[i], {});
    }

    int index = indexObject("soundTest");
    if(index >= 0 && l.physObjects[index])
    {
        BulletObject* bo = l.physObjects[index];
        bo->body()->setCollisionFlags(bo->body()->getCollisionFlags() |
                                      btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

        bo->body()->setUserIndex(102);
    }

    vector<int> indexs = indexObjects("soundTest2");
    for(int i : indexs)
    {
        BulletObject* bo = l.physObjects[i];
        bo->body()->setCollisionFlags(bo->body()->getCollisionFlags() |
                                      btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

        bo->body()->setUserIndex(103);
    }
    indexs = indexObjects("soundTest3");
    for(int i : indexs)
    {
        BulletObject* bo = l.physObjects[i];
        bo->body()->setCollisionFlags(bo->body()->getCollisionFlags() |
                                      btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

        bo->body()->setUserIndex(104);
    }

    auto lpVec = LightProbeUtils::importProbe("sceneInside_specprobe.xml");
    for(auto lp : lpVec)
    {
        level().levelScene->scene.add<interface::LightInstance>(LightProbeUtils::genLightProbe(lp));
    }
}

void Level1::update(float time)
{
//    vector<int> redObj = indexObjects("red");
//    bool allOk = true;

//    for(size_t i=0 ; i<redObj.size() ; ++i)
//    {
//        if(level().objects[redObj[i]].meshInstance->matrix().translation().x() > -2.2)
//            allOk = false;
//    }

//    //if(allOk)
//        setEnablePortal(true, level().objects[_indexPortal].meshInstance);
}

void Level1::prepareEnter()
{

}

