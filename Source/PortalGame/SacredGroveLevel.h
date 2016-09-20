#ifndef SACREDGROVELEVEL_H
#define SACREDGROVELEVEL_H

#include "PortalGame/LevelSystem.h"
#include "bullet/BulletEngine.h"

class SacredGroveBase : public LevelInterface
{
public:
    SacredGroveBase(int index, LevelSystem* system, BulletEngine&);

protected:
    BulletEngine& _physEngine;

    bool updateArtifact(const LevelSystem::GameObject&, int indexSlot, float height, float time);
};

class SacredGroveAux : public SacredGroveBase
{
public:
    SacredGroveAux(int index, LevelSystem* system, BulletEngine&, std::string color);
    void init() override;
    void update(float) override;

protected:
    std::string _color;
    int _indexArtifact, _indexSlot;
    bool _isArtifactBright = true;  
};

class SacredGroveMain : public SacredGroveBase
{
public:
    SacredGroveMain(int index, LevelSystem* system, BulletEngine&);
    void init() override;
    void update(float) override;

protected:
    int _indexButtons[3];
    int _indexColumns[3];
    BulletObject* _buttonObj[3];
    interface::MeshInstance* _buttonInst[3];
    int _activeButton = -1;

    int _indexPortals[5]; // in,out,red,white,blue
    bool _artifactOnSlot[3] = {false};
    bool _allActive = false;

    resource::SoundAsset _buttonSound;
    resource::SoundAsset _warp;
};


#endif // SACREDGROVELEVEL_H
