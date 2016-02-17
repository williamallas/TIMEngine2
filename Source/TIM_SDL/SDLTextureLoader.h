#ifndef SDLTEXTURELOADER_H
#define SDLTEXTURELOADER_H

#include "resource/TextureLoader.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
    class SDLTextureLoader : public resource::TextureLoader
    {
    public:
        SDLTextureLoader() = default;

        ubyte* loadImage(const std::string&, ImageFormat&) const;
        ubyte* loadImageArray(const vector<std::string>&, ImageFormat&) const;
        vector<ubyte*> loadImageCube(const vector<std::string>&, ImageFormat&) const;

    private:
    };

}
#include "MemoryLoggerOff.h"

#endif // SDLTEXTURELOADER_H
