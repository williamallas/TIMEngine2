#include "Mesh.h"
#include "interface/ShaderPool.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{

Mesh::Element::Element(const Geometry& g, float roughness, float metallic, const vec4& color, float specular) : _geometry(g)
{
    setRoughness(roughness);
    setMetallic(metallic);
    setSpecular(specular);
    setEmissive(0);
    setColor(color);
    setTextureScale(1);
}

Mesh::Element::Element()
{
    setDefault();
}

Mesh::Element::Element(const Geometry& g) : _geometry(g)
{
    setDefault();
}


Mesh::Element& Mesh::Element::operator=(const Element& e)
{
    _state = e._state;
    _geometry = e._geometry;
    _enable = e._enable;
    _castShadow = e._castShadow;
    for(int i=0 ; i<3 ; ++i)
        setTexture(e.texture(i), i);

    std::memcpy(&_userDefinedMaterial, &(e._userDefinedMaterial), sizeof(renderer::DummyMaterial));
    return *this;
}

void Mesh::Element::setTexture(const Texture& t, uint index)
{
    index = std::min(index,2u);
    _textures[index] = t;

    if(t.isNull()) _mat.texures[index] = 0;
    else _mat.texures[index] = t.texture()->handle();

    flushMat();
}

void Mesh::Element::setDefault()
{
    _enable = 2;
    _castShadow = true;
    _state = renderer::DrawState();
    _state.setShader(ShaderPool::instance().get("gPass"));

    for(int i=0 ; i<3 ; ++i)
        setTexture(Texture(), i);

    setRoughness(0.8);
    setMetallic(0);
    setSpecular(0.1);
    setEmissive(0);
    setColor(vec4::construct(0.7));
    setTextureScale(1);
}

void Mesh::Element::flushMat()
{
    _mat.header.y() = 0;
    for(int i=0 ; i<3 ; ++i)
    {
        if(!_textures[i].isNull())
            _mat.header.y() = i+1;
    }
}

}
}
