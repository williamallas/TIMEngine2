#include "TextureLoader.h"
#include "Image.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace resource
{

TextureLoader* textureLoader = nullptr;

unsigned char* TextureLoader::toGLFormat(unsigned char* ptr, unsigned char* out_ptr, const ImageFormat& format)
{
    if(!out_ptr)
        out_ptr = new unsigned char[format.size.x()*format.size.y()*format.nbComponent];

    for(size_t i=0 ; i<format.size.y() ; ++i)
    {
        memcpy(out_ptr + i*format.size.x()*format.nbComponent,
               ptr + (format.size.y()-i-1)*format.size.x()*format.nbComponent,
               format.size.x()*format.nbComponent);
    }

    //delete[] ptr;

    return out_ptr;
}

float* TextureLoader::toGLFormat(float* ptr, float* out_ptr, const ImageFormat& format)
{
    if(!out_ptr)
        out_ptr = new float[format.size.x()*format.size.y()*format.nbComponent];

    for(uint i=0 ; i<format.size.x() ; ++i)
        for(uint j=0 ; j<format.size.y() ; ++j)
    {
        memcpy(out_ptr + (j*format.size.x()+i)*format.nbComponent,
               ptr + (i*format.size.y()+j)*format.nbComponent,
               format.nbComponent*sizeof(float));
    }

    //delete[] ptr;
    return out_ptr;
}

ubyte* TextureLoader::loadImageArray(const vector<std::string>& files, ImageFormat& format) const
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

        memcpy(data+i*format.size.x()*format.size.y()*format.nbComponent, dat, format.size.x()*format.size.y()*format.nbComponent);
        delete[] dat;
        //toGLFormat(dat, data+i*format.size.x()*format.size.y()*format.nbComponent, format);
    }
    return data;
}

vector<ubyte*> TextureLoader::loadImageCube(const vector<std::string>& files, ImageFormat& format) const
{
    vector<ubyte*> result;

    for(size_t i=0 ; i<files.size() ; ++i)
    {
        ubyte* dat=loadImage(files[i], format);
        if(!dat)
            return result;

        //ubyte* tmp = new ubyte[format.size.x()*format.size.y()*format.nbComponent];
        //toGLFormat(dat, tmp, format);
        resource::Image img;
        img.buildFromData(dat, format);

        switch(i)
        {
            case 0: img.transform(imat2::ROT_270()); break;
            case 1: img.transform(imat2::ROT_90()); break;
            case 2: /*img.transform(imat2::ROT_180());*/ break;
            case 3: img.transform(imat2::ROT_180()); break;
            default: break;
        }

        result.push_back(img.cloneData(format.nbComponent));
        delete[] dat;
    }

    return result;
}

}
}
