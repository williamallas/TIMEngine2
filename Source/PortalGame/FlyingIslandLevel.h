#ifndef FLYINGISLANDLEVEL_H
#define FLYINGISLANDLEVEL_H

#include "PortalGame/LevelSystem.h"
#include "bullet/BulletEngine.h"
#include "PortalGame/BetweenSceneStruct.h"

class FlyingIslandLevel : public LevelInterface
{
public:
    FlyingIslandLevel(int index, LevelSystem* system, BulletEngine&, Sync_Ocean_FlyingIsland_PTR);
    virtual ~FlyingIslandLevel();

    void init() override;
    void update(float) override;
    void prepareEnter() override;
    void beforeLeave() override;

private:
    BulletEngine& _physEngine;

    Sync_Ocean_FlyingIsland_PTR _syncBoat;
    bool _enterWithBoat = true;

    struct Elevator
    {
        int elevatorObject;
        int elevatorObjectArrival;
        int indexElevator;
        int indexButton[2];
        float activateTime[2] = {0};

        int state = 0; // 0 init, 1 moving, 2 arrived
        float distance = 0;
    };
    vector<Elevator> _elevators;

    resource::SoundAsset _buttonSound;
};

#endif // FLYINGISLANDLEVEL_H
