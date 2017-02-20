#ifndef MESH_H
#define MESH_H

#include "core.h"
#include "renderer/renderer.h"
#include "renderer/DrawState.h"
#include "interface/Geometry.h"
#include "interface/Texture.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
    class Mesh
    {
    public:

        class Element
        {
        public:
            Element();
            Element(const Geometry& g);
            Element(const Geometry& g, float roughness, float metallic, const vec4& color = vec4::construct(1), float specular=0.5);

            Element(const Element& e) { *this = e; }
            Element& operator=(const Element&);

            void setRoughness(float r) { _mat.parameter[0] = r; }
            void setMetallic(float m) { _mat.parameter[1] = m; }
            void setSpecular(float s) { _mat.parameter[2] = s; }
            void setEmissive(float e) { _mat.parameter[3] = e; }
            void setColor(const vec4& c) { _mat.color = packColor(c[0],c[1],c[2],c[3]); }
            void setTextureScale(float x) { _mat.scale = static_cast<uint>(x * 1000); }

            void setMaterial(const vec4& m) { _mat.parameter = m; }

            float roughness() const { return _mat.parameter[0]; }
            float metallic() const { return _mat.parameter[1]; }
            float specular() const { return _mat.parameter[2]; }
            float emissive() const { return _mat.parameter[3]; }
            float textureScale() const { return _mat.scale / 1000.f; }

            vec4 color() const
            {
                float rgba[4];
                unpackColor(_mat.color, rgba);
                return vec4(rgba[0],rgba[1],rgba[2],rgba[3]);
            }

            template <class T>
            void copyUserDefinedMaterial(const T&);

            void resetMaterial();

            void setGeometry(const Geometry& g) { _geometry = g; }
            const Geometry& geometry() const { return _geometry; }

            void setEnable(int e) { _enable = e; }
            int isEnable() const { return _enable; }

            bool castShadow() const { return _castShadow; }
            void setCastShadow(bool b) { _castShadow = b; }

            bool cubemapAffected() const { return _mat.bitFlags > 0; }
            void setCubemapAffected(bool b){ _mat.bitFlags = b?1:0; }

            void setTexture(const Texture& t, uint index);
            Texture texture(uint index) const { index = std::min(index,2u); return _textures[index]; }

            renderer::DrawState& drawState() { return _state; }
            const renderer::DrawState& drawState() const { return _state; }

            const renderer::DummyMaterial& dummyMaterial() const { return _userDefinedMaterial; }

        private:
            Geometry _geometry;
            Texture _textures[3];

            union
            {
                renderer::DummyMaterial _userDefinedMaterial;
                renderer::Material _mat;
            };


            renderer::DrawState _state;
            int _enable = 2;
            bool _castShadow = true;


            void setDefault();
            void flushMat();
        };

        Mesh() = default;
        Mesh(const Element& e)
        {
            _elements.push_back(e);
            _initialVolume = e.geometry().volume();
        }

        Mesh(const Mesh&) = default;
        Mesh(Mesh&&) = default;
        Mesh& operator=(Mesh&&) = default;
        Mesh& operator=(const Mesh&) = default;

        ~Mesh() = default;

        Mesh& addElement(const Element& e)
        {
            if(_elements.empty())
                _initialVolume = e.geometry().volume();
            else
                _initialVolume = _initialVolume.max(e.geometry().volume());

            _elements.push_back(e);
            return *this;
        }

        Mesh& clear()
        {
            _elements.clear();
            _initialVolume = Sphere();
            return *this;
        }

        void flushVolume()
        {
            vector<Element> tmp;
            std::swap(tmp, _elements);
            clear();
            for(const Element& e : tmp)
                addElement(e);
        }

        Element& element(uint index) { return _elements[index]; }
        const Element& element(uint index) const { return _elements[index]; }

        bool empty() const { return _elements.empty(); }
        uint nbElements() const { return _elements.size(); }

        void setInitialVolume(const Sphere& s) { _initialVolume = s; }
        const Sphere& initialVolume() const { return _initialVolume; }

    private: 
        vector<Element> _elements;
        Sphere _initialVolume;
    };

    template <class T>
    void Mesh::Element::copyUserDefinedMaterial(const T& dat)
    {
        memcpy(&_userDefinedMaterial, &dat, sizeof(renderer::DummyMaterial));
    }

    inline void Mesh::Element::resetMaterial()
    {
        setDefault();
    }
}
}
#include "MemoryLoggerOff.h"

#endif // MESH_H
