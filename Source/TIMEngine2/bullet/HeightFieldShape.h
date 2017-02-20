#ifndef HEIGHTFIELDSHAPE_H
#define HEIGHTFIELDSHAPE_H

#include "core.h"

#include "btBulletCollisionCommon.h"
#include "ImageAlgorithm.h"

namespace tim
{

template<int UNSUDED=0>
static btHeightfieldTerrainShape* createHeightFieldShape(vec3 size, const ImageAlgorithm<float>& img)
{
    size /= vec3(img.size().x()-1, img.size().y()-1,1);
    btHeightfieldTerrainShape* shape = new btHeightfieldTerrainShape(img.size().x(), img.size().y(),
                                                                     img.data(),
                                                                     1,
                                                                     2, // z up
                                                                     true, // float data
                                                                     false);
    shape->setLocalScaling(btVector3(size.x(), size.y(), size.z()));
    return shape;
}

}

#endif // HEIGHTFIELDSHAPE_H
