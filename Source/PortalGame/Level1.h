#ifndef LEVEL1_H
#define LEVEL1_H

#include "PortalGame/LevelSystem.h"

class Level1 : public LevelInterface
{
public:
    Level1(int index, LevelSystem* system);

    void init() override;
    void prepareEnter() override;
    void update(float) override;

private:
    int _indexPortal;
};

#endif // LEVEL1_H
