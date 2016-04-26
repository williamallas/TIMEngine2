#ifndef QTTEXTURELOADER_H
#define QTTEXTURELOADER_H

#include "resource/TextureLoader.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    class QtTextureLoader : public resource::TextureLoader
    {
    public:
        QtTextureLoader() = default;
        ubyte* loadImage(const std::string&, ImageFormat&) const override;
    };
}
#include "MemoryLoggerOff.h"

#endif // QTTEXTURELOADER_H
