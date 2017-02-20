#ifndef FLYINGISLANDLEVEL_H
#define FLYINGISLANDLEVEL_H

#include "PortalGame/LevelSystem.h"
#include "bullet/BulletEngine.h"
#include "PortalGame/BetweenSceneStruct.h"
#include "ConnectFourIA.h"

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
        bool needToWin = false;

        int state = 0; // 0 init, 1 moving, 2 arrived
        float distance = 0;
    };
    vector<Elevator> _elevators;

    vector<int> _cubes;
    int _nbCloseCube = 0;

    struct Ring
    {
        int indexObject;
        bool enabled;
        std::unique_ptr<BulletObject> collObj;
    };
    btConvexShape* _ringShape;

    vector<Ring> _rings;
    int _portal1Index = -1;

    resource::SoundAsset _buttonSound, _warp, _connectFourSound;

    void testCubeOnRing(BulletObject*);
    bool _ringsEnabled = false;

    struct ConnectFourGame
    {
        ConnectFourIA ia;
        tim::interface::MeshInstance* jetons[ConnectFourIA::GRID_X][ConnectFourIA::GRID_Y] = {{nullptr}};
        vec3 toGo[ConnectFourIA::GRID_X][ConnectFourIA::GRID_Y];
        vec3 scale;
        mat4 transformation;
        interface::Mesh coins[2];

        void reset(interface::SimpleScene&);
        void update(float);
        mat4 getTransFromCoord(uivec2) const;
    };
    ConnectFourGame _4game;
    bool _hasWin = false;

    int _indexButtons[7];
    std::future<int> _futureMove;
    bool _firstTurn = true;
    bool _isFutureReady=false;

    const float DELAY_PLAY = 2;
    float _delayBeforeNextPlay = DELAY_PLAY;
    float _delayResetGame = -1;

    void resetButtons();
};

#endif // FLYINGISLANDLEVEL_H
