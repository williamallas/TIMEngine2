#ifndef PLAN_H_INCLUDED
#define PLAN_H_INCLUDED

#include "Vector.h"
#include "Matrix.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    class Plan
    {
    public:
        Plan() {}

        Plan(const vec3& p1, const vec3& p2, const vec3& p3)
        { setPlan(p1, p2, p3); }

        Plan(const vec3& point, const vec3& normal)
        { setPlan(point, normal); }

        Plan(const vec4& equ)
        { setPlan(equ); }

        template <size_t N>
        float findComponent(const vec3&) const;

        const vec4& plan() const { return _plan; }

        void setPlan(const vec3& point, const vec3& normal)
        {
            vec3 n = normal.normalized();
            _plan = {n.x(), n.y(), n.z(), -n.dot(point)};
        }

        void setPlan(const vec4& equ)
        {
            _plan = equ;
        }

        void setPlan(const vec3& p1, const vec3& p2, const vec3& p3)
        { setPlan(p2, (p1 - p2).cross(p3 - p2)); }

        float distance(const vec3& p) const
        { return (_plan.x()*p.x() + _plan.y()*p.y() + _plan.z()*p.z() + _plan.w()); }

        Plan& transform(const mat4& mat)
        {
             vec3 p(0,0,0);
             if(_plan[0] != 0) p[0] = -_plan[3] / _plan[0];
             else if(_plan[1] != 0) p[1] = -_plan[3] / _plan[1];
             else if(_plan[2] != 0) p[2] = -_plan[3] / _plan[2];

             p = mat*p;
             vec3 n = mat.to<3>() * _plan.to<3>();

             setPlan(p, n);

             return *this;
        }

        Plan transformed(const mat4& mat) const
        {
            Plan p = *this;
            return p.transform(mat);
        }

    private:
        vec4 _plan = {0,0,1,0};

    };

    template <size_t N>
    float Plan::findComponent(const vec3& v) const
    {
        float res = -_plan[3];
        for(uint i=0 ; i<3 ; ++i)
        {
            if(i != N)
                res -= _plan[i]*v[i];
        }

        return res / _plan[N];
    }
}
}
#include "MemoryLoggerOn.h"

#endif // PLAN_H_INCLUDED
