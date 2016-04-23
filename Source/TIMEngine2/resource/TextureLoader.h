#ifndef TEXTURELOADER_H_INCLUDED
#define TEXTURELOADER_H_INCLUDED

#include "core/core.h"
#include "ImageAlgorithm.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace resource
{
    class TextureLoader
    {
    public:
        struct ImageFormat
        {
            uivec2 size;
            size_t nbComponent;
        };

        TextureLoader()=default;
        virtual ~TextureLoader()=default;

        virtual ubyte* loadImage(const std::string&, ImageFormat&) const = 0;
        virtual ubyte* loadImageArray(const vector<std::string>&, ImageFormat&) const = 0;
        virtual vector<ubyte*> loadImageCube(const vector<std::string>&, ImageFormat&) const = 0;

        static ubyte* toGLFormat(ubyte*, ubyte*, const ImageFormat&);
        static float* toGLFormat(float*, float*, const ImageFormat&);
    };

    extern TextureLoader* textureLoader;
}
}
#include "MemoryLoggerOff.h"

#endif // TEXTURELOADER_H_INCLUDED
