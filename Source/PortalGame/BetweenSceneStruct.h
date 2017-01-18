#ifndef BETWEENSCENE_STRUCT_H
#define BETWEENSCENE_STRUCT_H

#include "interface/MeshInstance.h"
#include <memory>

using namespace tim;

struct Sync_Ocean_FlyingIsland
{
    interface::MeshInstance* boatOcean = nullptr;
    interface::MeshInstance* boatFI = nullptr;
    vec3 arrival;
    vec3 dir;
    float remainingDist;
};

using Sync_Ocean_FlyingIsland_PTR = std::shared_ptr<Sync_Ocean_FlyingIsland>;

#endif // BETWEENSCENE_STRUCT_H

