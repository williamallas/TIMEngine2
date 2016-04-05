#include "Frustum.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{

void Frustum::buildCameraFrustum(const Camera& camera, size_t maskPlan)
{
    float tang = tan(toRad(camera.fov)*0.5) ;
    float nw = camera.clipDist.x() * tang;
    float nh = nw / camera.ratio;
    float fw = camera.clipDist.y()  * tang;
    float fh = fw / camera.ratio;

    vec3 nc,fc,X,Y,Z;

    Y = (camera.dir-camera.pos).normalized();
    X = camera.up.cross(Y).normalized();
	Z = Y.cross(X);

    nc = (Y*camera.clipDist.x()) + camera.pos;
    fc = (Y*camera.clipDist.y()) + camera.pos;

    vec3 ntl = nc - X*nw + Z*nh;
    vec3 ntr = nc + X*nw + Z*nh;
    vec3 nbl = nc - X*nw - Z*nh;
    vec3 nbr = nc + X*nw - Z*nh;

    vec3 ftl = fc - X*fw + Z*fh;
    vec3 ftr = fc + X*fw + Z*fh;
    vec3 fbl = fc - X*fw - Z*fh;
    vec3 fbr = fc + X*fw - Z*fh;

    _plans.clear();

    if(!(maskPlan & BUILD_MASK(FrustumPlan::LEFT)))
        _plans.push_back(Plan(ntl,nbl,fbl));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::RIGHT)))
        _plans.push_back(Plan(nbr,ntr,fbr));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::UP)))
        _plans.push_back(Plan(ntr,ntl,ftl));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::DOWN)))
        _plans.push_back(Plan(nbl,nbr,fbr));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::NEAR)))
        _plans.push_back(Plan(ntl,ntr,nbr));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::FAR)))
        _plans.push_back(Plan(ftr,ftl,fbl));
}

void Frustum::buildCameraFrustum(const mat4& invProjView,
                                 size_t maskPlan)
{
    _plans.clear();
    vec3 ntl = vec3(invProjView*vec4(1,1,-1,1));
    vec3 ntr = vec3(invProjView*vec4(-1,1,-1,1));
    vec3 nbl = vec3(invProjView*vec4(1,-1,-1,1));
    vec3 nbr = vec3(invProjView*vec4(-1,-1,-1,1));

    vec4 ftl_ = invProjView*vec4(1,1,1,1);
    vec4 ftr_ = invProjView*vec4(-1,1,1,1);
    vec4 fbl_ = invProjView*vec4(1,-1,1,1);
    vec4 fbr_ = invProjView*vec4(-1,-1,1,1);

    vec3 ftl = vec3(ftl_ / ftl_.w());
    vec3 ftr = vec3(ftr_ / ftr_.w());
    vec3 fbl = vec3(fbl_ / fbl_.w());
    vec3 fbr = vec3(fbr_ / fbr_.w());

    if(!(maskPlan & BUILD_MASK(FrustumPlan::LEFT)))
        _plans.push_back(Plan(ntl,nbl,fbl));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::RIGHT)))
        _plans.push_back(Plan(nbr,ntr,fbr));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::UP)))
        _plans.push_back(Plan(ntr,ntl,ftl));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::DOWN)))
        _plans.push_back(Plan(nbl,nbr,fbr));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::NEAR)))
        _plans.push_back(Plan(ntl,ntr,nbr));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::FAR)))
        _plans.push_back(Plan(ftr,ftl,fbl));
}

void Frustum::buildOrthoFrustum(float l, float r, float b, float t, float n, float f,
                                const mat4& view_matrix, size_t maskPlan)

{
    _plans.clear();
    mat3 m3 = view_matrix.to<3>();
    mat4 m4=view_matrix.inverted();
    m3.invert();

    if(!(maskPlan & BUILD_MASK(FrustumPlan::LEFT)))
        _plans.push_back(Plan(m4*vec3(l,0,0), m3*vec3(1,0,0)));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::RIGHT)))
        _plans.push_back(Plan(m4*vec3(r,0,0), m3*vec3(-1,0,0)));

    if(!(maskPlan & BUILD_MASK(FrustumPlan::UP)))
        _plans.push_back(Plan(m4*vec3(0,t,0), m3*vec3(0,-1,0)));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::DOWN)))
        _plans.push_back(Plan(m4*vec3(0,b,0), m3*vec3(0,1,0)));

    if(!(maskPlan & BUILD_MASK(FrustumPlan::NEAR)))
        _plans.push_back(Plan(m4*vec3(0,0,-n), m3*vec3(0,0,-1)));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::FAR)))
        _plans.push_back(Plan(m4*vec3(0,0,-f), m3*vec3(0,0,1)));
}

void Frustum::buildOrthoFrustum(float l, float r, float b, float t, float n, float f,
                                const vec3& pos, const vec3& dir, const vec3& up, size_t maskPlan)
{
    _plans.clear();
    vec3 Y = (dir-pos).normalized();
	vec3 X = up.cross(Y).normalized();
	vec3 Z = Y.cross(X);

	if(!(maskPlan & BUILD_MASK(FrustumPlan::LEFT)))
        _plans.push_back(Plan(pos + X*l, X));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::RIGHT)))
        _plans.push_back(Plan(pos + X*r, -X));

    if(!(maskPlan & BUILD_MASK(FrustumPlan::UP)))
        _plans.push_back(Plan(pos + Z*t, -Z));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::DOWN)))
        _plans.push_back(Plan(pos + Z*b, Z));

    if(!(maskPlan & BUILD_MASK(FrustumPlan::NEAR)))
        _plans.push_back(Plan(pos + Y*n, Y));
    if(!(maskPlan & BUILD_MASK(FrustumPlan::FAR)))
        _plans.push_back(Plan(pos + Y*f, -Y));
}

Intersection Frustum::collide(const Sphere& s) const
{
    Intersection result = INSIDE;
	float distance;

	for(size_t i=0; i<_plans.size(); ++i)
	{
		distance = _plans[i].distance(s.center());

        if (distance < -s.radius())
			return OUTSIDE;
		else if (distance < s.radius())
			result =  INTERSECT;
	}

    return result;
}

Intersection Frustum::collide(const Box& b) const
{
    Intersection result = INSIDE;
    for(uint i=0; i < _plans.size(); ++i)
    {
		if(_plans[i].distance(getBoxVertexP(b, _plans[i].plan().down<1>())) < 0)
			return OUTSIDE;
		else if(_plans[i].distance(getBoxVertexN(b, _plans[i].plan().down<1>())) < 0)
            result = INTERSECT;
	}

	return result;
}

bool Frustum::collide(const vec3& p) const
{
    for(uint i=0; i < _plans.size(); ++i)
    {
		if(_plans[i].distance(p) < 0)
			return false;
	}
	return true;
}

vec3 Frustum::getBoxVertexP(const Box& b, const vec3& n) const
{
    vec3 result;
    for(size_t i=0 ; i<3 ; ++i)
    {
        if (n[i] > 0) result.set(b.box()[i][1], i);
        else result.set(b.box()[i][0], i);
    }

	return result;
}

vec3 Frustum::getBoxVertexN(const Box& b, const vec3& n) const
{
    vec3 result;
    for(size_t i=0 ; i<3 ; ++i)
    {
        if (n[i] < 0) result.set(b.box()[i][1], i);
        else result.set(b.box()[i][0], i);
    }

	return result;
}

}
}
