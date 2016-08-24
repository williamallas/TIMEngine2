#include "SDLTextureLoader.h"

#include <SDL_image.h>

#include "core/Matrix.h"
#include "resource/Image.h"

#include "MemoryLoggerOn.h"
namespace tim
{

ubyte* SDLTextureLoader::loadImage(const std::string& file, ImageFormat& format) const
{
    SDL_Surface* img = IMG_Load(file.c_str());
    if(!img)
        return nullptr;

    SDL_LockSurface(img);

    format.nbComponent = 4;
    format.size = { (size_t)img->w, (size_t)img->h };

    SDL_Surface* converted = SDL_ConvertSurfaceFormat(img, SDL_BYTEORDER == SDL_BIG_ENDIAN ? SDL_PIXELFORMAT_RGBA8888 : SDL_PIXELFORMAT_ABGR8888, 0);

    SDL_LockSurface(converted);

    //ubyte* pixels = new ubyte[format.size.x()*format.size.y()*format.nbComponent];
    ubyte* pixels = TextureLoader::toGLFormat((ubyte*)converted->pixels, nullptr, format);
    //memcpy(pixels, converted->pixels, format.size.x()*format.size.y()*format.nbComponent);

    SDL_UnlockSurface(converted);
    SDL_UnlockSurface(img);

    SDL_FreeSurface(converted);
    SDL_FreeSurface(img);

    return pixels;
}

}
