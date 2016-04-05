#include "MeshInstance.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{

void MeshInstance::setMatrix(const mat4& m)
{
    _model = m;
    Sphere s = _mesh.initialVolume();
    s.transform(m);
    setVolume(s);
}

void MeshInstance::setMesh(const Mesh& m)
{
    _mesh = m;
    setMatrix(_model);
}

}
}
