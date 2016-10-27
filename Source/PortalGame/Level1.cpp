#include "Level1.h"
#include "interface/ShaderPool.h"

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

    renderer::LightContextRenderer::Light liparam;
    vector<std::string> cubeMap(6);
    cubeMap[0] = "skybox/test/red.png";
    cubeMap[1] = "skybox/test/blue.png";
    cubeMap[2] = "skybox/test/yellow.png";
    cubeMap[3] = "skybox/test/purple.png";
    cubeMap[4] = "skybox/test/cyan.png";
    cubeMap[5] = "skybox/test/green.png";
    liparam.tex = renderer::IndirectLightRenderer::loadAndProcessSkybox(cubeMap, interface::ShaderPool::instance().get("processSpecularCubeMap")).second;
    liparam.position = {0,0,1};
    liparam.radius = 1.5;
    liparam.type = renderer::LightContextRenderer::Light::SPECULAR_PROB;

    auto& li = level().levelScene->scene.add<interface::LightInstance>(liparam);

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

