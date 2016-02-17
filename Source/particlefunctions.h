#ifndef PARTICLEFUNCTIONS
#define PARTICLEFUNCTIONS

#include "interface/Particle.h"

using namespace tim;
using namespace tim::interface;
using namespace tim::core;

/*
void particleUpdater_simpleGravity(Particle& p, float time, int)
{
    p.vel -= vec3(0,0,-10*time);
}

void particleEmiter_simpleEmitter(Particle& p, float time, int)
{
    p.position = vec3(0,0,0);
    p.lifeTime = 3;
    p.vel = Particle::createConeVel()
}
*/

struct TestParticle
{
    void create(Particle& p, Rand& r)
    {
        //p.assignParameter(bornParticleParam,r);
        p.position = vec3(0,0,0);//emitterPosition;
        p.other = vec3(0,0,0);
        Particle::createConeVel(p, 0, r,bornParticleParam);
        p.initialPosition = emitterPosition;
        p.vel += emitterVelocity;
    }

    void update(Particle& p, float time, Rand&)
    {
        float coef = 1.f-p.bornTime / p.initialLife;
        p.vel += vec3(0,0,-2)*time;
        p.vel - p.vel*0.1*time;
        p.color = interpolate(vec3(0,0,0), p.initialColor, coef);
        p.alpha_rotation.x() = coef*0.5+0.5;
        p.other += p.vel*time;
        if(p.vel.length2() > 4)
            p.vel.resize(2);
        p.position = p.other + p.initialPosition - emitterPosition;
    }

    vec3 emitterPosition, emitterVelocity;
    ParticleBornParameter bornParticleParam;
};

#endif // PARTICLEFUNCTIONS

