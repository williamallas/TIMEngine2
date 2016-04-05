#ifndef IMAGE_H
#define IMAGE_H

#include "core/core.h"
#include "TextureLoader.h"
#include "core/Matrix.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace resource
{

    class Image
    {
    public:
        Image() = default;
        Image(const uivec2&);
        ~Image();

        Image(const Image&);
        Image& operator=(const Image&);

        Image(Image&&);
        Image& operator=(Image&&);

        void setPixel(const vec4&, const uivec2&);
        void setPixel(const vec3&, const uivec2&);
        vec4 pixel(const uivec2&) const;

        void setPixel(const ubvec4&, const uivec2&);
        void setPixel(const ubvec3&, const uivec2&);
        ubvec4 pixel_ubyte4(const uivec2&) const;

        vec4 pixel(vec2) const; // bilinear interpolation

        uint* detachData();
        ubyte* cloneData(size_t nbComponent) const;

        template <int N>
        Image* reduceMinMax(const uivec2&) const;

        const uivec2& size() const;
        bool empty() const;

        std::string str() const;

        void fill(const vec4&);
        Image& convertToBW();
        Image& seuilBW(float, float);
        Image& blur3x3();
        Image& unpackFloat();
        Image& transpose();
        Image& transform(const imat2&);

        bool exportBmp(const std::string&) const;
        Image& buildFromData(unsigned char*, const TextureLoader::ImageFormat&);

        size_t memoryUsage() const;

    private:
        uint* _data=nullptr;
        uivec2 _size;

        void build(const uivec2&);

        vec4 safeRead(const ivec2&) const;
    };

    inline bool Image::empty() const { return _data == nullptr; }

    inline size_t Image::memoryUsage() const { return _size.dot(_size) * sizeof(uint); }

    template <int N>
    Image* Image::reduceMinMax(const uivec2& s) const
    {
        if(_size.x() % s.x() != 0 || _size.y() % s.y() != 0)
            return nullptr;

        uivec2 sizePatch = { _size.x() / s.x() , _size.y() / s.y() };

        Image * res = new Image(s);

        for(uint i=0 ; i<s.x() ; ++i) for(uint j=0 ; j<s.y() ; ++j)
        {
            ubvec2 min_max = {255,0};
            for(uint i_intern=0 ; i_intern < sizePatch.x() ; ++i_intern) for(uint j_intern=0 ; j_intern < sizePatch.y() ; ++j_intern)
            {
                uint x = i*sizePatch.x() + i_intern;
                uint y = j*sizePatch.y() + j_intern;
                ubyte p = pixel_ubyte4({x,y})[N];
                min_max.x() = std::min(min_max.x(), p);
                min_max.y() = std::max(min_max.y(), p);
            }

            res->setPixel(ubvec3(min_max.x(), min_max.y(), 0), uivec2(i,j));
        }

        return res;
    }

}
}
#include "MemoryLoggerOff.h"

#endif // IMAGE_H
