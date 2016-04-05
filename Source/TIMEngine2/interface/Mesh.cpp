#include "Mesh.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{

Mesh::Element::Element(const Geometry& g, float roughness, float metallic, const vec4& color, float specular) : _geometry(g)
{
    setRougness(roughness);
    setMetallic(metallic);
    setSpecular(specular);
    setColor(color);
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
    for(int i=0 ; i<3 ; ++i)
        setTexture(e.texture(i), i);

    setRougness(e.roughness());
    setMetallic(e.metallic());
    setSpecular(e.specular());
    setColor(e.color());
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
    _state = renderer::DrawState();
    for(int i=0 ; i<3 ; ++i)
        setTexture(Texture(), i);

    setRougness(1);
    setMetallic(0);
    setSpecular(0.5);
    setColor(vec4::construct(0.5));
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
