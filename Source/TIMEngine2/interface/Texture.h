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

    public:
        using resource::Asset<renderer::Texture>::Asset;

    protected:
        renderer::Texture* texture() const { return _ptr.get(); }
    };
}
}
#include "MemoryLoggerOff.h"

#endif // interfaceTEXTURE_H
