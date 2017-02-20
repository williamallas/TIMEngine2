#include "Particle.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{

void Particle::createPointRandVel(Particle& p, float, Rand& random)
{
    p.position = vec3(0,0,0);
    p.lifeTime = 3;
    p.color = vec3(random.next_f(), random.next_f(), random.next_f()).saturated();
    p.sizeXY_Z = vec3(vec2::construct(random.next_f()*0.01+0.01), 0);
    p.alpha_rotation.x() = 1;

    float x=random.next_f()*PI;
    float a=sinf(x);
    float b=random.next_f()*PI*2;
    p.vel = vec3(a*cosf(b), a*sinf(b), cosf(x))*random.next_f();
}

void Particle::simpleGravity(Particle& p, float time, Rand&)
{
    p.vel += vec3(0,0,-2)*time;
    p.position += p.vel*time;
}

void Particle::simpleVel(Particle& p, float time, Rand&)
{
    p.position += p.vel*time;
}
void Particle::createConeVel(Particle& p, float, Rand& random, const ParticleBornParameter& param)
{
    p.position=vec3(0,0,0);
    float longitude = 2*PI*random.next_f();
    float colatitude = interpolate(param.userParam1.x(), param.userParam1.y(), random.next_f())*PI;
    float sin_colatitude = sinf(colatitude);
    p.vel = vec3(sin_colatitude*cosf(longitude), sin_colatitude*sinf(longitude), cosf(colatitude));
    p.assignParameter(param, random);

}

void Particle::assignParameter(const ParticleBornParameter& param, Rand& random)
{
    lifeTime = random.next_f(param.lifeTime);

    vec4 col = interpolate(param.color_min, param.color_max, random.next_f());
    color = vec3(col);
    alpha_rotation.x() = col.w();
    sizeXY_Z = vec3(interpolate(param.size_min, param.size_max, random.next_f()), 0);
    vel.resize(interpolate(param.vel.x(), param.vel.y(), random.next_f()));
}

}
}
