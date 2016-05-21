#ifndef GEOMETRYSHAPE_H
#define GEOMETRYSHAPE_H

#include "core.h"
#include "interface/Geometry.h"
#include <btBulletDynamicsCommon.h>

namespace tim
{

    class GeometryShape
    {
    public:
        struct MeshInstance
        {
            interface::Geometry mesh;
            mat4 mat;
        };
        static btBvhTriangleMeshShape* genStaticGeometryShape(const vector<MeshInstance>&);
    };
}

#endif // GEOMETRYSHAPE_H
