#include "LevelSystem.h"
#include "MultipleSceneHelper.h"
#include "resource/AssetManager.h"
#include "openAL/Source.hpp"
using namespace resource;

#include "MemoryLoggerOn.h"

renderer::Texture::GenTexParam LevelSystem::defaultTexParam;

LevelSystem::LevelSystem(BulletEngine& ph, Listener& listener, Controller& controller, HmdSceneView& hmdView)
    : _physEngine(ph), _listener(listener), _controller(controller), _hmdView(hmdView)
{
    defaultTexParam = interface::Texture::genParam(true,true,true, 4);
}

LevelSystem::~LevelSystem()
{
    for(auto ptr : _levelStrategy)
    {
        delete ptr;
    }
}

int LevelSystem::nbLevels() const
{
    return (int)_levels.size();
}

void LevelSystem::addLevel(const Level& lvl)
{
    _levels.push_back({lvl, true});
    _levelStrategy.push_back(nullptr);
}

void LevelSystem::setStrategy(LevelInterface* strat, int index)
{
    _levelStrategy[index] = strat;
}

void LevelSystem::registerGameObject(int indexLevel, int indexObj, std::string name)
{
    _accessibleObjects.push_back({name, indexLevel, indexObj});
}

Option<LevelSystem::GameObject> LevelSystem::getGameObject(std::string name)
{
    for(AccessibleObject& o : _accessibleObjects)
    {
        if(o.name == name)
        {
            GameObject go;
            go.object = &_levels[o.indexParentLevel].first.objects[o.indexObject];
            go.physObject = _levels[o.indexParentLevel].first.physObjects[o.indexObject];
            go.sceneIndex = go.physObject ? go.physObject->indexWorld() : -1;
            return go;
        }
    }
    return Option<LevelSystem::GameObject>();
}

vector<LevelSystem::GameObject> LevelSystem::getGameObjects(std::string name)
{
    vector<LevelSystem::GameObject> objs;
    for(AccessibleObject o : _accessibleObjects)
    {
        if(o.name == name)
        {
            GameObject go;
            go.object = &_levels[o.indexParentLevel].first.objects[o.indexObject];
            go.physObject = _levels[o.indexParentLevel].first.physObjects[o.indexObject];
            go.sceneIndex = go.physObject ? go.physObject->indexWorld() : -1;
            objs.push_back(go);
        }
    }
    return objs;
}

void LevelSystem::initAll()
{
    for(uint i=0 ; i < _levels.size() ; ++i)
    {
        if(_levels[i].second)
        {
            _levels[i].second = false;
            if(_levelStrategy[i])
                _levelStrategy[i]->init();
        }
    }
}

void LevelSystem::changeLevel(int index)
{
    if(_curLevel == index)
        return;

    if(_curLevel >= 0)
    {
        manageTraversableObjOnSwitch(_levels[index].first.levelScene);
        if(_levelStrategy[_curLevel])
            _levelStrategy[_curLevel]->beforeLeave();
    }

    if(_levels[index].second)
    {
        _levels[index].second = false;
        _curLevel = index;

        if(_levelStrategy[index])
            _levelStrategy[index]->init();

    }
    _curLevel = index;

    if(_levelStrategy[index])
    {
        _levelStrategy[index]->prepareEnter();

        if(_curSoundName != _levelStrategy[index]->_soundName)
        {
            if(_curAmbientSound)
                _curAmbientSound->stop();

            if(_levelStrategy[index]->_ambientSound)
                _levelStrategy[index]->_ambientSound->play();
            _curSoundName = _levelStrategy[index]->_soundName;
            _curAmbientSound = _levelStrategy[index]->_ambientSound;
        }
    }
}

LevelSystem::Level& LevelSystem::getLevel(int index)
{
    return _levels[index].first;
}

void LevelSystem::update(float time)
{

    if(_levelStrategy[_curLevel])
        _levelStrategy[_curLevel]->update(time);

    auto it=_allPortalTaversableObj.begin();
    while(it != _allPortalTaversableObj.end())
    {
        PortalTraversableObject& x = *it;

        switch(x.state)
        {
            case PortalTraversableObject::NEW:
            {
                mat4 offset;
                auto scene_portal = _portalsHelper->closestPortal(x.instIn->volume(), offset);

                if(scene_portal.second && std::find(x.forbiddenPortals.begin(), x.forbiddenPortals.end(), scene_portal.second) != x.forbiddenPortals.end())
                {
                    #warning CHANGE_FORBIDEN_PORTAL_REACTION
                    vec3 v = x.instIn->volume().center() - scene_portal.second->volume().center();
                    v.z() = 0;
                    v.resize(5);
                    x.physObj->body()->applyCentralForce(btVector3(v.x(), v.y(), v.z()));
                    break;
                }

                if(scene_portal.first)
                {
                    x.state = PortalTraversableObject::ENGAGED_IN;
                    x.instOut = &scene_portal.first->scene.add<interface::MeshInstance>(x.instIn->mesh(), offset * x.instIn->matrix());
                    x.sceneOut = scene_portal.first;
                    x.offset = x.inv_offset = offset;
                    x.inv_offset.invert();
                    x.portal = scene_portal.second;
                }
            }
            x.lastPos = x.instIn->volume().center();
            break;

            case PortalTraversableObject::ENGAGED_IN:
            {
                int crossed = _portalsHelper->hasCrossedPortal(x.instIn->volume().center(), x.lastPos, x.portal,
                                                               std::max(x.instIn->volume().radius(), 0.1f));
                if(crossed == -1)
                {
                    x.sceneOut->scene.remove(*x.instOut);
                    x.sceneOut = nullptr;
                    x.instOut = nullptr;
                    x.portal = nullptr;
                    x.state = PortalTraversableObject::NEW;
                }
                else if(crossed == 0)
                {
                    x.instOut->setMatrix(x.offset*x.instIn->matrix());
                }
                else
                {
                    SceneMotionState<interface::MeshInstance>* mt = dynamic_cast<SceneMotionState<interface::MeshInstance>*>(x.physObj->body()->getMotionState());
                    if(mt) mt->setSceneObject(*x.instOut);
                    x.physObj->setMotionState(nullptr);

                    BulletObject* old = x.physObj;

                    x.physObj = new BulletObject(mt, old->body()->getCollisionShape(), 1.f / old->body()->getInvMass());
                    x.physObj->body()->setFriction(old->body()->getFriction());
                    x.physObj->body()->setRestitution(old->body()->getRestitution());
                    x.physObj->body()->setRollingFriction(old->body()->getRollingFriction());
                    x.physObj->body()->setCcdMotionThreshold(old->body()->getCcdMotionThreshold());
                    x.physObj->body()->setCcdSweptSphereRadius(old->body()->getCcdSweptSphereRadius());

                    vec3 v(old->body()->getLinearVelocity().getX(), old->body()->getLinearVelocity().getY(), old->body()->getLinearVelocity().getZ());
                    v = x.offset.to<3>() * v;
                    x.physObj->body()->setLinearVelocity(btVector3(v.x(), v.y(), v.z()));

                    v = vec3(old->body()->getAngularVelocity().getX(), old->body()->getAngularVelocity().getY(), old->body()->getAngularVelocity().getZ());
                    v = x.offset.to<3>() * v;
                    x.physObj->body()->setAngularVelocity(btVector3(v.x(), v.y(), v.z()));

                    x.physObj->body()->setUserIndex(old->body()->getUserIndex());
                    delete old;

                    _physEngine.addObject(x.physObj, getLevelIndex(x.sceneOut), old->colMask(), old->colWithMask());
                    x.state = PortalTraversableObject::ENGAGED_OUT;

                    if(x.indexObject >= 0)
                    {
                        getLevel(_curLevel).physObjects[x.indexObject] = x.physObj;
                        getLevel(_curLevel).objects[x.indexObject].meshInstance = x.instOut;
                    }
                }
            }
            x.lastPos = x.instIn->volume().center();
            break;

            case PortalTraversableObject::ENGAGED_OUT:
            {
                x.instIn->setMatrix(x.inv_offset*x.instOut->matrix());

                int crossed = _portalsHelper->hasCrossedPortal(x.instIn->volume().center(), x.lastPos, x.portal,
                                                               std::max(x.instIn->volume().radius(), 0.1f));

                if(crossed == -1) // remove from current world
                {
                    getLevel(_curLevel).levelScene->scene.remove(*x.instIn);
                    x.instIn = nullptr;
                    x.state = PortalTraversableObject::OUT;
                }
                else if(crossed == 0)
                {
                    x.lastPos = x.instIn->volume().center();
                }
                else
                {
                    SceneMotionState<interface::MeshInstance>* mt = dynamic_cast<SceneMotionState<interface::MeshInstance>*>(x.physObj->body()->getMotionState());
                    if(mt) mt->setSceneObject(*x.instIn);
                    x.physObj->setMotionState(nullptr);
                    BulletObject* old = x.physObj;

                    x.physObj = new BulletObject(mt, old->body()->getCollisionShape(), 1.f / old->body()->getInvMass());
                    x.physObj->body()->setFriction(old->body()->getFriction());
                    x.physObj->body()->setRestitution(old->body()->getRestitution());
                    x.physObj->body()->setRollingFriction(old->body()->getRollingFriction());
                    x.physObj->body()->setCcdMotionThreshold(old->body()->getCcdMotionThreshold());
                    x.physObj->body()->setCcdSweptSphereRadius(old->body()->getCcdSweptSphereRadius());

                    vec3 v(old->body()->getLinearVelocity().getX(), old->body()->getLinearVelocity().getY(), old->body()->getLinearVelocity().getZ());
                    v = x.inv_offset.to<3>() * v;
                    x.physObj->body()->setLinearVelocity(btVector3(v.x(), v.y(), v.z()));

                    v = vec3(old->body()->getAngularVelocity().getX(), old->body()->getAngularVelocity().getY(), old->body()->getAngularVelocity().getZ());
                    v = x.inv_offset.to<3>() * v;
                    x.physObj->body()->setAngularVelocity(btVector3(v.x(), v.y(), v.z()));

                    x.physObj->body()->setUserIndex(old->body()->getUserIndex());

                    delete old;

                    _physEngine.addObject(x.physObj, getLevel(_curLevel).indexScene, old->colMask(), old->colWithMask());
                    x.state = PortalTraversableObject::ENGAGED_IN;
                    x.lastPos = x.instIn->volume().center();

                    if(x.indexObject >= 0)
                    {
                        getLevel(_curLevel).physObjects[x.indexObject] = x.physObj;
                        getLevel(_curLevel).objects[x.indexObject].meshInstance = x.instIn;
                    }
                }

            }break;

            case PortalTraversableObject::OUT:
            {
                vec3 v = x.inv_offset * x.instOut->volume().center();
                float distOut = std::max(DIST_OUT, x.instOut->volume().radius());
                if((v-x.portal->volume().center()).length2() > distOut*distOut)
                {
                    if(x.sceneOut == getLevel(_curLevel).levelScene)
                    {
                        x.state = PortalTraversableObject::NEW;
                        x.instIn = x.instOut;
                        x.instOut = nullptr;
                        x.sceneOut = nullptr;
                        x.portal = nullptr;
                    }
                    else
                    {
                      _inacessiblePortalTaversableObj.push_back(x);
                      _allPortalTaversableObj.erase(it++);
                      continue;
                    }
                }
                else
                {
                    int crossed = _portalsHelper->hasCrossedPortal(v,v, x.portal,
                                                                   std::max(x.instOut->volume().radius(), 0.1f));

                    if(crossed == 0) // now close enough
                    {
                        x.instIn = &getLevel(_curLevel).levelScene->scene.add<interface::MeshInstance>(x.instOut->mesh(), x.inv_offset * x.instOut->matrix());
                        x.state = PortalTraversableObject::ENGAGED_OUT;
                    }
                }
                x.lastPos = v;
            }break;
        }
        ++it;
    }
}

void LevelSystem::setEnablePortal(bool b, interface::MeshInstance* inst)
{
    if(_portalsHelper)
        _portalsHelper->setEnableEdge(b, *_levels[_curLevel].first.levelScene, inst);
}

void LevelSystem::registerPortalTraversableObject(int indexObject, interface::MeshInstance* inst, BulletObject* phys, int index,
                                                  const vector<interface::MeshInstance*>& forbiddenPortals)
{
    if(index != _curLevel)
    {
        PortalTraversableObject obj;
        obj.forbiddenPortals = forbiddenPortals;
        obj.indexObject = indexObject;
        obj.instOut = inst;
        obj.physObj = phys;
        obj.sceneOut = getLevel(index).levelScene;
        _inacessiblePortalTaversableObj.push_back(obj);
    }
    else
    {
        PortalTraversableObject obj;
        obj.forbiddenPortals = forbiddenPortals;
        obj.indexObject = indexObject;
        obj.instIn = inst;
        obj.physObj = phys;
        obj.state = PortalTraversableObject::NEW;
        obj.lastPos = inst->volume().center();
        _allPortalTaversableObj.push_back(obj);
    }

}

int LevelSystem::getLevelIndex(interface::Scene* sc) const
{
    for(size_t i=0 ; i<_levels.size() ; ++i)
    {
        if(_levels[i].first.levelScene == sc)
            return (int)i;
    }
    return -1;
}

void LevelSystem::manageTraversableObjOnSwitch(interface::Scene* sceneTo)
{
    std::list<PortalTraversableObject> newInnacessible;
    auto it=_allPortalTaversableObj.begin();
    while(it != _allPortalTaversableObj.end())
    {
        PortalTraversableObject& x = *it;
        switch(x.state)
        {
            case PortalTraversableObject::NEW:
            {
                mat4 offset;
                auto scene_portal = _portalsHelper->closestPortal(Sphere(x.instIn->volume().center(), std::max(DIST_OUT, x.instIn->volume().radius())), offset);

                if(scene_portal.first && x.sceneOut == sceneTo) // become out
                {
                    auto sc_portal = _portalsHelper->communicatingPortal(getLevel(_curLevel).levelScene, scene_portal.second);

                    if(sc_portal.first == sceneTo)
                    {
                        x.state = PortalTraversableObject::OUT;
                        x.instOut = x.instIn;
                        x.instIn = nullptr;

                        x.sceneOut = getLevel(_curLevel).levelScene;
                        x.offset = x.inv_offset = offset;
                        x.offset.invert();
                        x.portal = sc_portal.second;
                        break;
                    }
                }

                x.instOut = x.instIn;
                x.sceneOut = getLevel(_curLevel).levelScene;
                x.instIn = nullptr;
                x.portal = nullptr;
                newInnacessible.push_back(x);
                _allPortalTaversableObj.erase(it++);

                continue;
            }

            case PortalTraversableObject::ENGAGED_IN:
            {
                auto sc_portal = _portalsHelper->communicatingPortal(getLevel(_curLevel).levelScene, x.portal);

                if(sc_portal.first == sceneTo && sc_portal.second)
                {
                    std::swap(x.instIn, x.instOut);
                    std::swap(x.offset, x.inv_offset);
                    x.portal = sc_portal.second;
                    x.sceneOut = getLevel(_curLevel).levelScene;
                    x.state = PortalTraversableObject::ENGAGED_OUT;
                    x.lastPos = x.inv_offset * x.lastPos;
                    break;
                }
                else
                {
                    x.sceneOut->scene.remove(*x.instOut);
                    x.instOut = x.instIn;
                    x.sceneOut = getLevel(_curLevel).levelScene;
                    x.instIn = nullptr;
                    x.portal = nullptr;
                    newInnacessible.push_back(x);
                    _allPortalTaversableObj.erase(it++);
                    continue;
                }
            }

            case PortalTraversableObject::ENGAGED_OUT:
            {
                auto sc_portal = _portalsHelper->communicatingPortal(getLevel(_curLevel).levelScene, x.portal);
                if(x.sceneOut == sceneTo && sc_portal.second)
                {
                    std::swap(x.instIn, x.instOut);
                    std::swap(x.offset, x.inv_offset);
                    x.portal = sc_portal.second;
                    x.sceneOut = getLevel(_curLevel).levelScene;
                    x.state = PortalTraversableObject::ENGAGED_IN;
                    x.lastPos = x.inv_offset * x.lastPos;
                    break;
                }
                else
                {
                    getLevel(_curLevel).levelScene->scene.remove(*x.instIn);
                    x.instIn = nullptr;
                    x.portal = nullptr;
                    newInnacessible.push_back(x);
                    _allPortalTaversableObj.erase(it++);
                    continue;
                }

            }


        case PortalTraversableObject::OUT:
            x.portal = nullptr;
            if(x.sceneOut == sceneTo)
            {
                x.state = PortalTraversableObject::NEW;
                x.instIn = x.instOut;
                x.instOut = nullptr;
            }
            else
            {
                newInnacessible.push_back(x);
                _allPortalTaversableObj.erase(it++);
                continue;
            }
            break;
        }

        ++it;
    }

    auto it2 = _inacessiblePortalTaversableObj.begin();
    while(it2 != _inacessiblePortalTaversableObj.end())
    {
        if(it2->sceneOut == sceneTo)
        {
            it2->instIn = it2->instOut;
            it2->instOut = nullptr;
            it2->sceneOut = nullptr;
            it2->state = PortalTraversableObject::NEW;
            _allPortalTaversableObj.push_back(*it2);
            _inacessiblePortalTaversableObj.erase(it2++);
        }
        else
            ++it2;
    }

    _inacessiblePortalTaversableObj.splice(_inacessiblePortalTaversableObj.end(), newInnacessible);
}

void LevelSystem::callDebug() {
    if(_levelStrategy[_curLevel])
        _levelStrategy[_curLevel]->callDebug();
}

vec3 LevelSystem::headPosition() const
{
    return _hmdView.cullingView().camera.pos;
}


/********************/
/** Levelinterface **/
/********************/

LevelSystem::Level& LevelInterface::level()
{
    return _system->getLevel(_index);
}

void LevelInterface::bindSound(BulletObject* bo, int indexSound)
{
    if(bo)
    {
        bo->body()->setCollisionFlags(bo->body()->getCollisionFlags() |
                                      btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

        bo->body()->setUserIndex(indexSound + 100);
    }
}

void LevelInterface::setEnablePortal(bool b, interface::MeshInstance* inst)
{
    _system->setEnablePortal(b, inst);
}

int LevelInterface::indexObject(std::string str)
{
    for(size_t i=0 ; i<level().objects.size() ; ++i)
    {
        if(level().objects[i].name == str)
            return i;
    }
    return -1;
}

vector<int> LevelInterface::indexObjects(std::string str)
{
    vector<int> res;
    for(size_t i=0 ; i<level().objects.size() ; ++i)
    {
        if(level().objects[i].name == str)
            res.push_back(i);
    }
    return res;
}

int LevelInterface::index() const
{
    return _index;
}

void LevelInterface::registerPortableTraversable(int indexObj, interface::MeshInstance* inst, BulletObject* o, const vector<std::string>& forbiddenP)
{
    vector<interface::MeshInstance*> fp;
    for(auto str : forbiddenP)
    {
        int indexP = indexObject(str);
        if(indexP >= 0)
            fp.push_back(level().objects[indexP].meshInstance);
    }
    _system->registerPortalTraversableObject(indexObj, inst, o, _index, fp);
}

void LevelInterface::registerGameObject(int index, std::string name)
{
    _system->registerGameObject(_index, index, name);
}

Option<LevelSystem::GameObject> LevelInterface::getGameObject(std::string name)
{
    return _system->getGameObject(name).value();
}

vector<LevelSystem::GameObject> LevelInterface::getGameObjects(std::string name)
{
    return _system->getGameObjects(name);
}

LevelSystem& LevelInterface::levelSystem()
{
    return *_system;
}

bool LevelInterface::collidePaddles(BulletObject* obj)
{
    if(!obj)
        return false;

    return levelSystem().controller().controllerInfo().leftHandPhys->collideWith(obj->body()).size() > 0 ||
           levelSystem().controller().controllerInfo().rightHandPhys->collideWith(obj->body()).size() > 0;
}

