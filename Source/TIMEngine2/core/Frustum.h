#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "core.h"
#include "Matrix.h"
#include "Plan.h"
#include "Camera.h"
#include "Intersection.h"
#include "Sphere.h"
#include "Box.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    class Frustum
    {
    public:
        Frustum() = default;
        ~Frustum() = default;

        enum FrustumPlan
        {
            LEFT=0,
            RIGHT,
            UP,
            DOWN,
            NEAR,
            FAR,
        };

        void buildCameraFrustum(const Camera&,
                                size_t maskPlanDiscard=0);

        void buildCameraFrustum(const mat4& invProjView,
                                size_t maskPlanDiscard=0);

        void buildOrthoFrustum(float l, float r, float b, float t, float n, float f,
                                const mat4& viewMat,
                                size_t maskPlanDiscard=0);

        void buildOrthoFrustum(float l, float r, float b, float t, float n, float f,
                                const vec3&, const vec3&, const vec3&, size_t maskPlanDiscard=0);

        void add(const Plan&);
        const Plan& plan(int) const;
        void clear();

        /* Collision*/
        Intersection collide(const Sphere&) const;
        Intersection collide(const Box&) const;
        bool collide(const vec3&) const;

    private:
        vector<Plan> _plans;

        vec3 getBoxVertexP(const Box&, const vec3&) const;
        vec3 getBoxVertexN(const Box&, const vec3&) const;
    };

    inline void Frustum::clear() { _plans.clear(); }
    inline void Frustum::add(const Plan& p) { _plans.push_back(p); }
    inline const Plan& Frustum::plan(int i) const { return _plans[i]; }
}
}
#include "MemoryLoggerOff.h"

#endif // FRUSTUM_H
