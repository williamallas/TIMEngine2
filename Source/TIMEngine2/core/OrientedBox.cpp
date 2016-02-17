#include "OrientedBox.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace core
{

OrientedBox::OrientedBox() : _matrix(mat4::IDENTITY()), _invMatrix(mat4::IDENTITY()), _isAligned(true) { }
OrientedBox::OrientedBox(const Box& box, const mat4& matrix) :
    _box(box), _matrix(matrix), _invMatrix(matrix.inverted()), _isAligned(matrix.to<3>() == mat3::IDENTITY()) { }

OrientedBox::~OrientedBox() { }

void OrientedBox::computeAxis(const Box& aabb, const mat4& mat, OrientedBoxAxis& box)
{
    vec3 c = aabb.center();
    box.center = mat*c;
    mat3 matrix3 = mat.down<1>();

    box.axis[0] = matrix3*vec3(aabb.box().x().y()-c.x(), 0, 0);
    box.axis[1] = matrix3*vec3(0, aabb.box().y().y()-c.y(), 0);
    box.axis[2] = matrix3*vec3(0, 0, aabb.box().z().y()-c.z());
}

std::string OrientedBox::str() const
{
    OrientedBoxAxis obb;
    computeAxis(_box, _matrix, obb);
    return "OBB(Center:"+obb.center.str()+";Axis:"+obb.axis[0].str()+";"+obb.axis[1].str()+";"+obb.axis[2].str()+")";
}

}
}
