#ifndef STARTLEVEL_H
#define STARTLEVEL_H

#include "PortalGame/LevelSystem.h"

class StartLevel : public LevelInterface
{
public:
    StartLevel(int index, LevelSystem* system);

    void init() override;
    void prepareEnter() override;
    void update(float) override;

private:
    int _indexPortal;
    int _indexStack[5];
    vector<int> _indexBalls;
    float _timeSinceOk = 0;
};

class Start2Level : public LevelInterface
{
public:
    Start2Level(int index, LevelSystem* system);

    void init() override;
    void prepareEnter() override;
    void update(float) override;

private:

};

#endif // STARTLEVEL_H
