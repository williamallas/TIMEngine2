#include "Sphere.h"
#include "Box.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{

Sphere::Sphere() { }
Sphere::Sphere(const vec3& center, float rad) : _center(center), _radius(rad) { }
Sphere::Sphere(const vec4& v) : _center(v.down<1>()), _radius(v.w()) { }
Sphere::Sphere(const Sphere& s) : _center(s._center), _radius(s._radius) { }

Sphere::~Sphere() { }

Box Sphere::toBox() const
{
    return Box(Vector3<vec2>(vec2(_center.x()-_radius, _center.x()+_radius),
                             vec2(_center.y()-_radius, _center.y()+_radius),
                             vec2(_center.z()-_radius, _center.z()+_radius)));
}

/* AABB */
bool Sphere::inside(const Box& box) const
{
    const vec3 hs = box.halfSize();
    float radius2=_radius*_radius;
    const vec3 bcenter = box.center();

    for(int i=-1 ; i<=1 ; i+=2)
        for(int j=-1 ; j<=1 ; j+=2)
            for(int k=-1 ; k<=1 ; k+=2)
    {
        if((_center - bcenter+hs*vec3(i,j,k)).length2() > radius2)
            return false;
    }

    return true;
}

bool Sphere::outside(const Box& box) const
{
    const vec3 minP = box.min();
    const vec3 maxP = box.max();

    vec3 closest = { _center.x() < minP.x() ? minP.x() : (_center.x() > maxP.x() ? maxP.x() : _center.x()),
                     _center.y() < minP.y() ? minP.y() : (_center.y() > maxP.y() ? maxP.y() : _center.y()),
                     _center.z() < minP.z() ? minP.z() : (_center.z() > maxP.z() ? maxP.z() : _center.z())
                   };

    if((_center - closest).length2() > _radius*_radius)
        return true;

    return false;
}

std::string Sphere::str() const
{
    std::string str = "Sphere(";
    str += _center.str() + std::string(",") + StringUtils(_radius).str() + ")";
    return str;
}

}
}
