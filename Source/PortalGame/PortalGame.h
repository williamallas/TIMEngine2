#ifndef PORTALGAME_H__
#define PORTALGAME_H__

#include "MultiSceneManager.h"
#include "PortalGame/Controller.h"
#include "OpenVR/HmdSceneView.h"
#include "OpenVR/VR_Device.h"

#include "PortalGame/BetweenSceneStruct.h"
#include "PortalGame/ForestLevel.h"
#include "PortalGame/Level1.h"
#include "PortalGame/SacredGroveLevel.h"
#include "PortalGame/OceanLevel.h"
#include "PortalGame/StartLevel.h"
#include "PortalGame/FlyingIslandLevel.h"

#include <utility>

class PortalGame
{
public:
    static bool contactCallBack(btManifoldPoint& cp, void* body0,void* body1);

    PortalGame(BulletEngine&, MultipleSceneHelper&, HmdSceneView&, VR_Device&, int startLevel = 0);

    void update(float time);

    int curSceneIndex() const;

    Controller& controllers();
    MultiSceneManager& multiSceneManager();
    LevelSystem& levelSystem();

    void setDebugControllerPose(vec3 v) { _debugControllerPos = v; }
    void popBoxDebug();

    enum SoundEffects
    {
        WOOD1 = 0, WOOD2, WOOD3,
        PLASTIC1, PLASTIC2,
        METAL1, METAL2, ARTIFACT1,
        SOUR,
        ROCK1,
        NB_EFFECTS
    };

protected:
    BulletEngine& _physEngine;
    MultipleSceneHelper& _multiSceneHelper;
    HmdSceneView& _hmdCamera;
    VR_Device& _vrDevice;
    Listener _listener;

    MultiSceneManager _multiScene;
    Controller _vrControllers;
    mat4 _lastL, _lastR;

    LevelSystem _levels;
    interface::XmlMeshAssetLoader _gameAssets;
    vector<resource::SoundAsset> _soundEffects;
    vector<Source*> _asyncSoundToPlay;
    std::map<std::pair<const btCollisionObject*,const btCollisionObject*>, int> _lastSoundPair;
    int _nbTotalSound = 0;
    int _clearSoundPairTimer=0;

    vec3 _debugControllerPos;
    int _frameId=0;

    void registerSoundCallBack();
    bool processContactPoint(btManifoldPoint& pt, const btCollisionObject* obj, btVector3 posContact);

};

inline Controller& PortalGame::controllers()
{
    return _vrControllers;
}

inline MultiSceneManager& PortalGame::multiSceneManager()
{
    return _multiScene;
}

inline LevelSystem& PortalGame::levelSystem()
{
    return _levels;
}

#endif // PORTALGAME_H
