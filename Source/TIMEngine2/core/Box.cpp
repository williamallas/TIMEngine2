#include "Box.h"
#include "Sphere.h"
#include "OrientedBox.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{

Box::Box() { }
Box::Box(const Vector<vec2, 3>& box) : _box(box) { }
Box::Box(const Box& box) { _box=box._box; }
Box::Box(const vec3& mini, const vec3& maxi) { setMin(mini); setMax(maxi); }

Box::~Box() { }

std::string Box::str() const
{
    std::string str="AABB(";
    str+=StringUtils(_box[0]).str()+",";
    str+=StringUtils(_box[1]).str()+",";
    str+=StringUtils(_box[2]).str()+")";
    return str;
}

Sphere Box::toSphere() const
{
    return Sphere(center(), min().length());
}

/* sphere */
bool Box::inside(const Sphere& s) const
{
    /* Check if center is outside */
//    for(size_t i=0 ; i<3 ; ++i)
//        if(s.center()[i] < _box[i].x()) return false;
//    for(size_t i=0 ; i<3 ; ++i)
//        if(s.center()[i] > _box[i].y()) return false;

    vec3 a = s.center(); a-=vec3(_box[0].x(), _box[1].x(), _box[2].x());
    vec3 b(_box[0].y(), _box[1].y(), _box[2].y()); b-=s.center();
    const float rad2 = s.radius()*s.radius();

    for(size_t i=0 ; i<3 ; i++)
    {
        if(a[i]<b[i])
        {
            if(a[i]*a[i] < rad2) return false;
        }
        else if(b[i]*b[i] < rad2) return false;
    }
    return true;
}

bool Box::outside(const Sphere& s) const
{
    vec3 closest = { s.center().x() < _box[0].x() ? _box[0].x() : (s.center().x() > _box[0].y() ? _box[0].y() : s.center().x()),
                     s.center().y() < _box[1].x() ? _box[1].x() : (s.center().y() > _box[1].y() ? _box[1].y() : s.center().y()),
                     s.center().z() < _box[2].x() ? _box[2].x() : (s.center().z() > _box[2].y() ? _box[2].y() : s.center().z())
                   };

    if((s.center() - closest).length2() > s.radius()*s.radius())
        return true;

    return false;
}

/* Obb */
bool Box::inside(const OrientedBox& obb) const
{
    if(obb.isAligned())
    {
        return inside(obb.box()+obb.matrix().translation());
    }
    OrientedBoxAxis axe;
    obb.computeAxis(obb.box(), obb.matrix(), axe);

    for(int i=-1 ; i<=1 ; i+=2)
        for(int j=-1 ; j<=1 ; j+=2)
            for(int k=-1 ; k<=1 ; k+=2)
    {
        if(outside(axe.axis[0]*i+axe.axis[1]*j+axe.axis[2]*k + axe.center))
            return false;
    }

    return true;
}

bool Box::outside(const OrientedBox& obb) const
{
    if(obb.isAligned())
    {
        return outside(obb.box()+obb.matrix().translation());
    }

    OrientedBoxAxis t_axis;
    OrientedBox::computeAxis(obb.box(), obb.matrix(), t_axis);
    if(testAABBAxis(*this, t_axis))
        return true;

    OrientedBox::computeAxis(*this, obb.inverseMatrix(), t_axis);
    if(testAABBAxis(obb.box(), t_axis))
        return true;

    return false;
}

bool Box::testAABBAxis(const Box& aabb, const OrientedBoxAxis& obb)
{
    vec3 aabbCenter = aabb.center();

    /* test of the x aligned obb axis */
    if(aabbCenter.x() < obb.center.x())
    {
        if( std::min(obb.axis[0].x()+obb.axis[1].x()+obb.axis[2].x(), std::min(obb.axis[0].x()+obb.axis[1].x()-obb.axis[2].x(),
            std::min(obb.axis[0].x()-obb.axis[1].x()+obb.axis[2].x(), std::min(obb.axis[0].x()-obb.axis[1].x()-obb.axis[2].x(),
            std::min(-obb.axis[0].x()+obb.axis[1].x()+obb.axis[2].x(), std::min(-obb.axis[0].x()+obb.axis[1].x()-obb.axis[2].x(),
            std::min(-obb.axis[0].x()-obb.axis[1].x()+obb.axis[2].x(), -obb.axis[0].x()-obb.axis[1].x()-obb.axis[2].x())))))))
           > aabbCenter.x()-obb.center.x() ) return true;
    }
    else
    {
        if( std::max(obb.axis[0].x()+obb.axis[1].x()+obb.axis[2].x(), std::max(obb.axis[0].x()+obb.axis[1].x()-obb.axis[2].x(),
            std::max(obb.axis[0].x()-obb.axis[1].x()+obb.axis[2].x(), std::max(obb.axis[0].x()-obb.axis[1].x()-obb.axis[2].x(),
            std::max(-obb.axis[0].x()+obb.axis[1].x()+obb.axis[2].x(), std::max(-obb.axis[0].x()+obb.axis[1].x()-obb.axis[2].x(),
            std::max(-obb.axis[0].x()-obb.axis[1].x()+obb.axis[2].x(), -obb.axis[0].x()-obb.axis[1].x()-obb.axis[2].x())))))))
           < aabbCenter.x()-obb.center.x() ) return true;
    }

    /* test of the y aligned obb axis */
    if(aabbCenter.y() < obb.center.y())
    {
        if( std::min(obb.axis[0].y()+obb.axis[1].y()+obb.axis[2].y(), std::min(obb.axis[0].y()+obb.axis[1].y()-obb.axis[2].y(),
            std::min(obb.axis[0].y()-obb.axis[1].y()+obb.axis[2].y(), std::min(obb.axis[0].y()-obb.axis[1].y()-obb.axis[2].y(),
            std::min(-obb.axis[0].y()+obb.axis[1].y()+obb.axis[2].y(), std::min(-obb.axis[0].y()+obb.axis[1].y()-obb.axis[2].y(),
            std::min(-obb.axis[0].y()-obb.axis[1].y()+obb.axis[2].y(), -obb.axis[0].y()-obb.axis[1].y()-obb.axis[2].y())))))))
           > aabbCenter.y()-obb.center.y() ) return true;
    }
    else
    {
        if( std::max(obb.axis[0].y()+obb.axis[1].y()+obb.axis[2].y(), std::max(obb.axis[0].y()+obb.axis[1].y()-obb.axis[2].y(),
            std::max(obb.axis[0].y()-obb.axis[1].y()+obb.axis[2].y(), std::max(obb.axis[0].y()-obb.axis[1].y()-obb.axis[2].y(),
            std::max(-obb.axis[0].y()+obb.axis[1].y()+obb.axis[2].y(), std::max(-obb.axis[0].y()+obb.axis[1].y()-obb.axis[2].y(),
            std::max(-obb.axis[0].y()-obb.axis[1].y()+obb.axis[2].y(), -obb.axis[0].y()-obb.axis[1].y()-obb.axis[2].y())))))))
           < aabbCenter.y()-obb.center.y() ) return true;
    }

    /* test of the z aligned obb axis */
    if(aabbCenter.z() < obb.center.z())
    {
        if( std::min(obb.axis[0].z()+obb.axis[1].z()+obb.axis[2].z(), std::min(obb.axis[0].z()+obb.axis[1].z()-obb.axis[2].z(),
            std::min(obb.axis[0].z()-obb.axis[1].z()+obb.axis[2].z(), std::min(obb.axis[0].z()-obb.axis[1].z()-obb.axis[2].z(),
            std::min(-obb.axis[0].z()+obb.axis[1].z()+obb.axis[2].z(), std::min(-obb.axis[0].z()+obb.axis[1].z()-obb.axis[2].z(),
            std::min(-obb.axis[0].z()-obb.axis[1].z()+obb.axis[2].z(), -obb.axis[0].z()-obb.axis[1].z()-obb.axis[2].z())))))))
           > aabbCenter.z()-obb.center.z() ) return true;
    }
    else
    {
        if( std::max(obb.axis[0].z()+obb.axis[1].z()+obb.axis[2].z(), std::max(obb.axis[0].z()+obb.axis[1].z()-obb.axis[2].z(),
            std::max(obb.axis[0].z()-obb.axis[1].z()+obb.axis[2].z(), std::max(obb.axis[0].z()-obb.axis[1].z()-obb.axis[2].z(),
            std::max(-obb.axis[0].z()+obb.axis[1].z()+obb.axis[2].z(), std::max(-obb.axis[0].z()+obb.axis[1].z()-obb.axis[2].z(),
            std::max(-obb.axis[0].z()-obb.axis[1].z()+obb.axis[2].z(), -obb.axis[0].z()-obb.axis[1].z()-obb.axis[2].z())))))))
           < aabbCenter.z()-obb.center.z() ) return true;
    }
    return false;
}

Intersection Box::collide(const OrientedBox& obb) const
{
    OrientedBoxAxis axis;
    obb.computeAxis(obb.box(), obb.matrix(), axis);
    bool findOutside=false;

    for(int i=-1 ; i<=1 ; i+=2)
        for(int j=-1 ; j<=1 ; j+=2)
            for(int k=-1 ; k<=1 ; k+=2)
    {
        if(outside(axis.axis[0]*i+axis.axis[1]*j+axis.axis[2]*k + axis.center))
        {
            findOutside = true;
            break;
        }
    }

    if(!findOutside)
        return INSIDE;

    if(testAABBAxis(*this, axis))
        return OUTSIDE;

    OrientedBox::computeAxis(*this, obb.inverseMatrix(), axis);
    if(testAABBAxis(obb.box(), axis))
        return OUTSIDE;

    return INTERSECT;
}

Box Box::operator+(const vec3& v) const
{
    return Box(vec3(_box[0].x(), _box[1].x(), _box[2].x())+v,
               vec3(_box[0].y(), _box[1].y(), _box[2].y())+v);
}


}
}
