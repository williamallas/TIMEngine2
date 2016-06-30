#ifndef SIMPLESCENE_H
#define SIMPLESCENE_H

#include "scene/BasicScene.h"
#include "scene/Transformable.h"
#include "Frustum.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
    using SimpleScene = scene::BasicScene<scene::Transformable>;

    struct CameraCulling
    {
        CameraCulling(const Camera& cam)
        {
            frustum.buildCameraFrustum(cam);
        }

        Frustum frustum;

        bool operator()(const scene::Transformable& t) const
        {
            return t.enabled() && frustum.collide(t.volume()) != Intersection::OUTSIDE;
        }
    };
    
    struct FrustumCulling
    {
        FrustumCulling(const Frustum& frust) : frustum(frust) {}
        Frustum frustum;

        bool operator()(const scene::Transformable& t) const
        {
            return t.enabled() && frustum.collide(t.volume()) != Intersection::OUTSIDE;
        }
    };

    struct RayCast
    {
        RayCast(const vec3& in_pos, const vec3& in_dir) : pos(in_pos), dir(in_dir.normalized()) {}
        vec3 pos, dir;

        bool operator()(const scene::Transformable& t) const
        {
            vec3 tmp;
            return t.enabled() && t.volume().collide(pos, dir, tmp);
        }
    };
}
}
#include "MemoryLoggerOff.h"

#endif // MESHINSTANCE_H
