#ifndef PARTICLE_H_INCLUDED
#define PARTICLE_H_INCLUDED

#include "core/core.h"
#include "core/Rand.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
    struct ParticleBornParameter;
    struct Particle
    {
        /* draw information */
        vec3 position;
        vec3 color = {1,1,1}; // send as normal
        vec2 alpha_rotation = {1,0}; // send as texCoord
        vec3 sizeXY_Z; // send as tangent

        /* important */
        float lifeTime=0; // if lifeTime < 0, discard the particle

        /*self managed */
        uint id;
        float bornTime=0;

        /* other */
        float mass=1;
        vec3 vel;

        /** Some particle function */
        static void createPointRandVel(Particle& p, float time, Rand& random);
        static void simpleGravity(Particle& p, float time, Rand& random);
        static void simpleVel(Particle& p, float time, Rand& random);

        static void createConeVel(Particle& p, float time, Rand& random, const ParticleBornParameter& param);

        void assignParameter(const ParticleBornParameter& p, Rand& random);
    };

    struct ParticleBornParameter
    {
        vec2 lifeTime={1,1};
        vec2 vel={1,1};
        vec4 color_min, color_max=vec4::construct(1);
        vec2 size_min=vec2(0.1,0.1), size_max=vec2(0.1,0.1);
        vec2 mass={1,1};

        vec4 userParam1;
    };
}
}
#include "MemoryLoggerOff.h"

#endif // PARTICULE_H_INCLDED
