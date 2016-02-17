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

    ubyte* pixels = nullptr;
    if(format.size.x() <= 1024 && format.size.y() <= 1024)
    {
        pixels = (unsigned char*)converted->pixels;
        converted->pixels = nullptr;
    }
    else
    {
        pixels = new ubyte[format.size.x()*format.size.y()*format.nbComponent];
        memcpy(pixels, converted->pixels, format.size.x()*format.size.y()*format.nbComponent);
    }

    SDL_UnlockSurface(converted);
    SDL_UnlockSurface(img);

    SDL_FreeSurface(converted);
    SDL_FreeSurface(img);

    return pixels;
}

ubyte* SDLTextureLoader::loadImageArray(const vector<std::string>& files, ImageFormat& format) const
{
    ubyte* data = nullptr;

    for(size_t i=0 ; i<files.size() ; ++i)
    {
        ubyte* dat=loadImage(files[i], format);

        if(!dat)
        {
            delete[] data;
            return nullptr;
        }

        if(!data)
            data = new ubyte[format.size.x()*format.size.y()*format.nbComponent*files.size()];

        toGLFormat(dat, data+i*format.size.x()*format.size.y()*format.nbComponent, format);
    }

    return data;
}

vector<ubyte*> SDLTextureLoader::loadImageCube(const vector<std::string>& files, ImageFormat& format) const
{
    //ubyte* data = nullptr;
    vector<ubyte*> result;

    for(size_t i=0 ; i<files.size() ; ++i)
    {
        ubyte* dat=loadImage(files[i], format);

        if(!dat)
            return result;

        ubyte* tmp = new ubyte[format.size.x()*format.size.y()*format.nbComponent];
        toGLFormat(dat, tmp, format);
        resource::Image img;
        img.buildFromData(tmp, format);

        switch(i)
        {
            case 0: img.transform(imat2::ROT_270()); break;
            case 1: img.transform(imat2::ROT_90()); break;
            case 2: /*img.transform(imat2::ROT_180());*/ break;
            case 3: img.transform(imat2::ROT_180()); break;
            default: break;
        }

        //ubyte* clonedData = img.cloneData(format.nbComponent);
        result.push_back(img.cloneData(format.nbComponent));

        //memcpy(data+i*format.size.x()*format.size.y()*format.nbComponent, clonedData, format.size.x()*format.size.y()*format.nbComponent);

        delete[] tmp;
        //delete[] clonedData;
    }

    return result;
}

}
