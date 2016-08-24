#ifndef AABBVOLUME_H
#define AABBVOLUME_H

#include "Vector.h"
#include "Intersection.h"
#include "Plan.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    class Sphere;
    class OrientedBox;
    struct OrientedBoxAxis;

    class Box
    {

    public:

        static Box computeBox(const real* ptr, uint size, uint stride=1);

        Box();
        Box(const Vector<vec2, 3>&);
        Box(const vec3&, const vec3&);
        Box(const Box&);
        ~Box();

        /* Getter and Setter */
        const Vector<vec2, 3>& box() const;
        Box& setX(const vec2&);
        Box& setY(const vec2&);
        Box& setZ(const vec2&);
        Box& setMin(const vec3&);
        Box& setMax(const vec3&);

        /* Utils */
        vec3 min() const;
        vec3 max() const;
        vec3 center() const;
        vec3 size() const;
        vec3 halfSize() const;
        Box max(const Box&) const;
        Sphere toSphere() const;

        /* Collision */

        bool inside(const Box&) const;
        bool outside(const Box&) const;

        bool inside(const Sphere&) const;
        bool outside(const Sphere&) const;

        bool inside(const vec3&) const;
        bool outside(const vec3&) const;

        bool inside(const OrientedBox&) const;
        bool outside(const OrientedBox&) const;

        template<class T> Intersection collide(const T&) const;
        Intersection collide(const OrientedBox&) const;

        Box operator+(const vec3&) const;

        Plan extractOptimalPlan() const;
        Box extractOriginAlignedBox() const;

        /* out */
        std::string str() const;
        friend std::ostream& operator<< (std::ostream& stream, const Box& t) { stream << t.str(); return stream;}

    private:
        Vector<vec2, 3> _box;

        static bool testAABBAxis(const Box&, const OrientedBoxAxis&);
    };


    /** Inline implementation */
    inline const Vector<vec2, 3>& Box::box() const { return _box; }
    inline Box& Box::setX(const vec2& v) { _box[0]=v; return *this; }
    inline Box& Box::setY(const vec2& v) { _box[1]=v; return *this; }
    inline Box& Box::setZ(const vec2& v) { _box[2]=v; return *this; }
    inline Box& Box::setMin(const vec3& v) { for(size_t i=0 ; i<3 ; ++i)_box[i].x()=v[i]; return *this; }
    inline Box& Box::setMax(const vec3& v) { for(size_t i=0 ; i<3 ; ++i)_box[i].y()=v[i]; return *this; }

    inline vec3 Box::min() const { return vec3(_box[0].x(), _box[1].x(), _box[2].x()); }
    inline vec3 Box::max() const { return vec3(_box[0].y(), _box[1].y(), _box[2].y()); }
    inline vec3 Box::center() const { return vec3(_box[0].x()+_box[0].y() , _box[1].x()+_box[1].y(), _box[2].x()+_box[2].y())*0.5; }
    inline vec3 Box::size() const { return max()-min(); }
    inline vec3 Box::halfSize() const { return size()*0.5; }

    inline Box Box::max(const Box& box) const
    {
        vec2 ret[] = { vec2(std::min(box._box[0].x(), _box[0].x()),std::max(box._box[0].y(), _box[0].y())),
                       vec2(std::min(box._box[1].x(), _box[1].x()),std::max(box._box[1].y(), _box[1].y())),
                       vec2(std::min(box._box[2].x(), _box[2].x()),std::max(box._box[2].y(), _box[2].y())) };
        return Box( Vector3<vec2> (ret[0], ret[1], ret[2]) );
    }

    /* box */
    inline bool Box::inside(const Box& box) const
    {
        for(size_t i=0 ; i<3 ; ++i)
            if(_box[i].y() < box._box[i].y()) return false;

        for(size_t i=0 ; i<3 ; ++i)
            if(_box[i].x() > box._box[i].x()) return false;

        return true;
    }

    inline bool Box::outside(const Box& box) const
    {
        for(size_t i=0 ; i<3 ; ++i)
            if(_box[i].x() > box._box[i].y()) return true;

        for(size_t i=0 ; i<3 ; ++i)
            if(_box[i].y() < box._box[i].x()) return true;

        return false;
    }

    /* point */
    inline bool Box::inside(const vec3& p) const
    {
        for(size_t i=0 ; i<3 ; ++i)
            if(p[i] < _box[i].x() || p[i] > _box[i].y())
                return false;

        return true;
    }

    inline bool Box::outside(const vec3& p) const { return !inside(p); }

    template<class T>
    Intersection Box::collide(const T& t) const
    {
        if(outside(t)) return OUTSIDE;
        else if(inside(t)) return INSIDE;
        else return INTERSECT;
    }
}
}
#include "MemoryLoggerOff.h"

#endif
