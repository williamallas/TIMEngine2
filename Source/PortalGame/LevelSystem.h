#ifndef LEVELSYSTEM_H
#define LEVELSYSTEM_H

#include "interface/XmlSceneLoader.h"
#include "bullet/BulletEngine.h"
#include "openAL/Listener.hpp"
#include "openAL/Source.hpp"
#include "Controller.h"
#include <list>

//#define AUTO_SOLVE

using namespace tim;      

class LevelInterface;
class MultipleSceneHelper;

class LevelSystem
{
public:
    using GraphicGameObject = interface::XmlSceneLoader::ObjectLoaded;
    struct Level
    {
        std::string name;
        interface::Scene* levelScene;
        int indexScene;

        vector<GraphicGameObject> objects;
        vector<BulletObject*> physObjects;
    };

    static renderer::Texture::GenTexParam defaultTexParam;

    LevelSystem(BulletEngine&, Listener&, Controller&);
    ~LevelSystem();

    Listener& listener() { return _listener; }
    Controller& controller() { return _controller; }

    void addLevel(const Level&);
    void setStrategy(LevelInterface* strat, int index);
    void setPortalHelper(MultipleSceneHelper* p) { _portalsHelper = p; }

    int nbLevels() const;
    int getCurLevelIndex() const { return _curLevel; }
    int getLevelIndex(interface::Scene*) const;

    void changeLevel(int);
    Level& getLevel(int);
    void setEnablePortal(bool, interface::MeshInstance*);

    void update(float);

    void registerPortalTraversableObject(int, interface::MeshInstance*, BulletObject*, int, const vector<interface::MeshInstance*>&);
    void registerGameObject(int indexLevel, int indexObj, std::string name);

    struct GameObject
    {
        GraphicGameObject* object;
        BulletObject* physObject;
        int sceneIndex;
    };

    Option<GameObject> getGameObject(std::string name);
    vector<GameObject> getGameObjects(std::string name);

    void callDebug();

protected:
    BulletEngine& _physEngine;
    Listener& _listener;
    Controller& _controller;
    vector<std::pair<Level, bool>> _levels;
    vector<LevelInterface*> _levelStrategy;

    std::string _curSoundName;
    Source* _curAmbientSound = nullptr;

    int _curLevel = -1;
    MultipleSceneHelper* _portalsHelper = nullptr;

    const float DIST_OUT = 1.5;
    struct PortalTraversableObject
    {
        int indexObject = -1;
        interface::MeshInstance* instIn = nullptr;
        interface::MeshInstance* instOut = nullptr;
        interface::Scene* sceneOut = nullptr;
        interface::MeshInstance* portal = nullptr;
        mat4 offset, inv_offset;
        BulletObject* physObj = nullptr;

        vector<interface::MeshInstance*> forbiddenPortals;

        enum { NEW, ENGAGED_IN, ENGAGED_OUT, OUT };
        int state = NEW;
        vec3 lastPos;
    };
    std::list<PortalTraversableObject> _allPortalTaversableObj;
    std::list<PortalTraversableObject> _inacessiblePortalTaversableObj;

    struct AccessibleObject
    {
        std::string name;
        int indexParentLevel;
        int indexObject;
    };
    std::vector<AccessibleObject> _accessibleObjects;

    void manageTraversableObjOnSwitch(interface::Scene*);
};

class LevelInterface
{
    friend class LevelSystem;
public:
    LevelInterface(int index, LevelSystem* system) : _index(index), _system(system) {}
    virtual ~LevelInterface() {}

    virtual void init() {}
    virtual void prepareEnter() {}
    virtual void update(float) {}
    virtual void beforeLeave() {}

    LevelSystem::Level& level();
    LevelSystem& levelSystem();
    int indexObject(std::string);
    vector<int> indexObjects(std::string);

    Option<LevelSystem::GameObject> getGameObject(std::string);
    vector<LevelSystem::GameObject> getGameObjects(std::string);

    void setEnablePortal(bool b, interface::MeshInstance*);
    int index() const;
    void registerPortableTraversable(int, interface::MeshInstance*, BulletObject*, const vector<std::string>&);
    void registerGameObject(int index, std::string name);

    bool collidePaddles(BulletObject*);

    void setAmbientSound(Source* src, std::string name) { _ambientSound = src; _soundName = name; }

    static void bindSound(BulletObject*, int);

    virtual void callDebug(){}

private:
    int _index;
    LevelSystem* _system;
    Source* _ambientSound = nullptr;
    std::string _soundName;
};

#endif // LEVELSYSTEM_H
