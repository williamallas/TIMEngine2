#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "Vector.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    struct Camera
    {
        vec3 pos, dir={0,1,0}, up={0,0,1};
        float fov=70, ratio=1;
        vec2 clipDist={1,100};

        vec3 computeCenter() const
        {
            return pos+(dir-pos).normalized()*(clipDist.y()-clipDist.x())*0.5;
        }
    };
}
}
#include "MemoryLoggerOff.h"


#endif // CAMERA_H_INCLUDED
