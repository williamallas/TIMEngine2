#include "BulletTerrainObject.h"

namespace tim
{

BulletTerrainObject::BulletTerrainObject(const vec2& position, float* hdata, const uivec2& res, const vec3& scale)
{
    _hdata = new float[res.dot(res)];
    vec2 minmax = {hdata[0],hdata[0]};
    for(uint i=0 ; i<res.x() ; ++i)for(uint j=0 ; j<res.y() ; ++j)
    {
        _hdata[j*res.x()+i] = hdata[i*res.y()+j];
        minmax.x() = std::min(minmax.x(), hdata[i*res.y()+j]);
        minmax.y() = std::max(minmax.y(), hdata[i*res.y()+j]);
    }

    _terrainShape = new btHeightfieldTerrainShape(res.x(), res.y(), _hdata, 1, minmax.x(), minmax.y(), 2, PHY_FLOAT, false);
    _terrainShape->setLocalScaling(btVector3(scale.x(), scale.y(), scale.z()));

    //_terrainShape->setUseZigzagSubdivision(true);
    //_terrainShape->setUseDiamondSubdivision(true);

    _object = new BulletObject(mat4::Translation({position.x(), position.y(), scale.z()*0.5f*(minmax[0]+minmax[1])}), _terrainShape, 0);
    _object->body()->setFriction(3);
    _object->body()->setRestitution(0.2);
}

BulletTerrainObject::~BulletTerrainObject()
{
    delete _object;
    delete _terrainShape;
    delete _hdata;
}

}
