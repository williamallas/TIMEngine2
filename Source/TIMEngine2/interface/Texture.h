#ifndef interfaceTEXTURE_H
#define interfaceTEXTURE_H

#include "resource/Asset.h"
#include "renderer/Texture.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
    class Mesh;
    //class Mesh::Element;
//    namespace pipeline {
//        class DeferredRendererNode;
//        class DirLightShadowNode;
//    }
}

namespace interface
{
    /** Immutable class */
    class Texture : public resource::Asset<renderer::Texture>
    {
        friend class Mesh;
        friend class LightInstance;

    public:
        using resource::Asset<renderer::Texture>::Asset;

        static renderer::Texture::GenTexParam genParam(bool repeat=true, bool linear=true, bool trilinear=true, int aniso=0)
        {
            renderer::Texture::GenTexParam param;
            param.anisotropy = aniso;
            param.depthMode = false;
            param.format = renderer::Texture::Format::RGBA8;
            param.linear = linear;
            param.nbLevels = -1;
            param.repeat = repeat;
            param.trilinear = trilinear;
            return param;
        }

        uint64_t handle() const { return _ptr->handle(); }

    protected:
        renderer::Texture* texture() const { return _ptr.get(); }
    };
}
}
#include "MemoryLoggerOff.h"

#endif // interfaceTEXTURE_H
