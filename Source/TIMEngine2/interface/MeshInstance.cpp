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
    bool sameVolume = m.initialVolume() == _mesh.initialVolume();
    _mesh = m;

    if(!sameVolume)
        setMatrix(_model);
}

void MeshInstance::attachUBO(uint id, uint index)
{
    if(_extraUbo.size() <= index)
        _extraUbo.resize(index+1);

    _extraUbo[index] = id;
}

const vector<uint>& MeshInstance::attachedUBO() const
{
    return _extraUbo;
}

void MeshInstance::clearAttachedUBO()
{
    _extraUbo.clear();
}

}
}
