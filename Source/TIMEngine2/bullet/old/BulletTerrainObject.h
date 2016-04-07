#ifndef BULLETTERRAINSHAPE_H
#define BULLETTERRAINSHAPE_H

#include "core/core.h"
#include "BulletObject.h"
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

namespace tim
{
    class BulletTerrainObject
    {
    public:
        BulletTerrainObject(const vec2&, float*, const uivec2&, const vec3&);
        virtual ~BulletTerrainObject();

        BulletObject* bulletObject() const { return _object; }

    private:
        btHeightfieldTerrainShape* _terrainShape;
        float* _hdata;

        BulletObject* _object;
    };
}

#endif // BULLETTERRAINSHAPE_H
