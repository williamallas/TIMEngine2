#ifndef SIMPLESCENE_H
#define SIMPLESCENE_H

#include "scene/SceneContainer.h"
#include "scene/Transformable.h"
#include "Frustum.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
    using SimpleScene = scene::SceneContainer<scene::Transformable>;

    struct CameraCulling
    {
        CameraCulling(const Camera& cam)
        {
            frustum.buildCameraFrustum(cam);
        }

        Frustum frustum;

        bool operator()(const scene::Transformable& t) const
        {
            return frustum.collide(t.volume()) != Intersection::OUTSIDE;
        }
    };
    
    struct FrustumCulling
    {
        FrustumCulling(const Frustum& frust) : frustum(frust) {}
        Frustum frustum;

        bool operator()(const scene::Transformable& t) const
        {
            return frustum.collide(t.volume()) != Intersection::OUTSIDE;
        }
    };
}
}
#include "MemoryLoggerOff.h"

#endif // MESHINSTANCE_H
