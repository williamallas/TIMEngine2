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

void Sphere::transform(const mat4& m)
{
    _center = m*_center;
    mat3 m3 = m.down<1>();
    _radius = std::max(std::max((m3*vec3(_radius,0,0)).length(), (m3*vec3(0,_radius,0)).length()),
                                (m3*vec3(0,0, _radius)).length());
}

bool Sphere::operator==(const Sphere& s) const
{
    return s._center == _center && s._radius == _radius;
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

Sphere Sphere::computeSphere(const real* ptr, uint size, uint stride)
{
    vec3 minV = vec3::construct(std::numeric_limits<float>::max()), maxV = vec3::construct(std::numeric_limits<float>::min());

    for(uint i=0 ; i<size ; ++i)
    {
        for(int j=0 ; j<3 ; ++j)
        {
            minV[j] = std::min(minV[j], ptr[j+i*stride]);
            maxV[j] = std::max(maxV[j], ptr[j+i*stride]);
        }
    }

    vec3 center = (minV+maxV) / 2;
    //center = {0,0,0};

    float minD = 0;
    for(uint i=0 ; i<size ; ++i)
    {
        vec3 v = {ptr[i*stride], ptr[i*stride+1], ptr[i*stride+2]};
        minD = std::max(minD, (v-center).length());
    }

    return Sphere(center, minD);
}

bool Sphere::collide(const vec3& o, const vec3& l, vec3& res) const
{

    vec3 diff = o - _center;
    float dotLD = l.dot(diff);

    float toSqrt = dotLD*dotLD - diff.length2() + _radius*_radius;

    if(toSqrt < 0)
        return false;

    float d = -dotLD - sqrtf(toSqrt);

    if(d < 0)
    {
        d = -dotLD + sqrtf(toSqrt);
        if(d < 0)
            return false;
    }

    res = o + l*d;
    return true;
}

}
}
