#include "Image.h"
#include <fstream>

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace resource
{

Image::Image(const uivec2& v)
{
    build(v);
    fill(vec4());
}

Image::~Image()
{
    delete[] _data;
}

Image::Image(const Image& img)
{
    if(!img.empty())
    {
        build(img.size());
        memcpy(_data, img._data, sizeof(uint)*_size.x()*_size.y());
    }
}

Image& Image::operator=(const Image& img)
{
    build(img.size());
    memcpy(_data, img._data, sizeof(uint)*_size.x()*_size.y());
    return *this;
}

Image::Image(Image&& img)
{
    _size = img._size;
    _data = img._data;
    img._data = nullptr;
    img._size = uivec2(0,0);
}

Image& Image::operator=(Image&& img)
{
    delete[] _data;
    _size = img._size;
    _data = img._data;
    img._data = nullptr;
    img._size = uivec2(0,0);
    return *this;
}

void Image::build(const uivec2& v)
{
    _size=v;
    delete[] _data;

    if(v.x()*v.y() > 0)
        _data = new uint[v.x()*v.y()];
    else _data = nullptr;
}

uint* Image::detachData()
{
    uint* old=_data;
    _data=nullptr;
    _size={0,0};
    return old;
}

ubyte* Image::cloneData(size_t nbComponent) const
{
    ubyte* buf = new ubyte[_size.x()*_size.y()*nbComponent];
    ubyte* srcBuf = reinterpret_cast<ubyte*>(_data);
    for(size_t i=0  ; i<_size.x() ; ++i)
    {
        for(size_t j=0  ; j<_size.y() ; ++j)
        {
            memcpy(&buf[i*_size.y()*nbComponent + j*nbComponent], &srcBuf[i*_size.y()*4 + j*4], nbComponent);
        }
    }

    return buf;
}

const uivec2& Image::size() const
{
    return _size;
}

void Image::setPixel(const vec4& col, const uivec2& v)
{
    uint p = (static_cast<uint>(col[3]*255.f) << 24) +
             (static_cast<uint>(col[2]*255.f) << 16) +
             (static_cast<uint>(col[1]*255.f) << 8)  +
              static_cast<uint>(col[0]*255.f);

    _data[v.x()*_size.y()+v.y()] = p;
}

void Image::setPixel(const vec3& col, const uivec2& v)
{
    uint p = (static_cast<uint>(col[2]*255.f) << 16) +
             (static_cast<uint>(col[1]*255.f) << 8) +
              static_cast<uint>(col[0]*255.f);

    _data[v.x()*_size.y()+v.y()] = p;
}

vec4 Image::pixel(const uivec2& v) const
{
    ubyte* p = reinterpret_cast<ubyte*>(&_data[v.x()*_size.y()+v.y()]);
    return { p[0]/255.f, p[1]/255.f, p[2]/255.f, p[3]/255.f };
}

void Image::setPixel(const ubvec4& col, const uivec2& v)
{
    uint p = (static_cast<uint>(col[3]) << 24) +
             (static_cast<uint>(col[2]) << 16) +
             (static_cast<uint>(col[1]) << 8)  +
              static_cast<uint>(col[0]);

    _data[v.x()*_size.y()+v.y()] = p;
}

void Image::setPixel(const ubvec3& col, const uivec2& v)
{
    uint p = (static_cast<uint>(col[2]) << 16) +
             (static_cast<uint>(col[1]) << 8) +
              static_cast<uint>(col[0]);

    _data[v.x()*_size.y()+v.y()] = p;
}

ubvec4 Image::pixel_ubyte4(const uivec2& v) const
{
    ubyte* p = reinterpret_cast<ubyte*>(&_data[v.x()*_size.y()+v.y()]);
    return { p[0], p[1], p[2], p[3] };
}

vec4 Image::pixel(vec2 v) const
{
    v *= vec2(_size.x()-1, _size.y()-1);
    vec2 floor_v = v;
    floor_v.apply(floorf);
    v -= floor_v;

    return interpolate2(safeRead({static_cast<int>(floor_v.x()), static_cast<int>(floor_v.y())}),
                        safeRead({static_cast<int>(floor_v.x()+1), static_cast<int>(floor_v.y())}),
                        safeRead({static_cast<int>(floor_v.x()), static_cast<int>(floor_v.y()+1)}),
                        safeRead({static_cast<int>(floor_v.x()+1), static_cast<int>(floor_v.y()+1)}),
                        v.x(), v.y());
}

std::string Image::str() const
{
    std::string res;
    for(size_t i=0 ; i<_size.x() ; ++i)
    {
        for(size_t j=0 ; j<_size.y() ; ++j)
        {
            res += pixel(uivec2(i,j)).str() + " ";
        }
        res += "\n";
    }
    return res;
}

void Image::fill(const vec4& col)
{
    uint p = (static_cast<uint>(col[3]*255.f) << 24) +
             (static_cast<uint>(col[2]*255.f) << 16) +
             (static_cast<uint>(col[1]*255.f) << 8)  +
              static_cast<uint>(col[0]*255.f);

    for(size_t i=0 ; i<_size.x()*_size.y() ; ++i)
        _data[i]=p;
}

Image& Image::convertToBW()
{
    for(size_t i=0 ; i<_size.x() ; ++i)
    {
        for(size_t j=0 ; j<_size.y() ; ++j)
        {
            char* b=reinterpret_cast<char*>(&_data[i*_size.y()+j]);
            setPixel(vec3::construct(*(b+2)/255.f), uivec2(i,j));
        }
    }
    return *this;
}

Image& Image::seuilBW(float seuil, float ds)
{
    for(size_t i=0 ; i<_size.x() ; ++i)
    {
        for(size_t j=0 ; j<_size.y() ; ++j)
        {
            vec3 c = vec3(pixel(uivec2(i,j)));
            if(c[0] <= seuil-ds)
                c = vec3();
            else if(c[0] >= seuil+ds)
                c = vec3(1,1,1);
            else
                c = vec3::construct(interpolateCos(0.f,1.f, (c[0]-seuil+ds)/(2*ds)));
            setPixel(c, uivec2(i,j));
        }
    }
    return *this;
}

vec4 Image::safeRead(const ivec2& v) const
{
    return pixel(uivec2(static_cast<uint>(std::min(std::max(v.x(),0),static_cast<int>(_size.x()-1))),
                        static_cast<uint>(std::min(std::max(v.y(),0),static_cast<int>(_size.y()-1)))));
}

Image& Image::blur3x3()
{
    for(int i=0 ; i<static_cast<int>(_size.x()) ; ++i)
    {
        for(int j=0 ; j<static_cast<int>(_size.y()) ; ++j)
        {
            vec4 c = safeRead({i-1,j-1})*0.25 + safeRead({i,j-1})*0.5 + safeRead({i+1,j-1})*0.25
                   + safeRead({i-1,j})*0.5 + safeRead({i,j}) + safeRead({i+1,j})*0.5
                   + safeRead({i-1,j+1})*0.25 + safeRead({i,j+1})*0.5 + safeRead({i+1,j+1})*0.25;

            c*=0.25;
            setPixel(c, uivec2(i,j));
        }
    }
    return *this;
}

Image& Image::unpackFloat()
{
    for(size_t i=0 ; i<_size.x() ; ++i)
    {
        for(size_t j=0 ; j<_size.y() ; ++j)
        {
            vec4 p=pixel(uivec2(i,j));
            uint color;
            ubyte* c=reinterpret_cast<ubyte*>(&color);
            for(size_t i=0 ; i<3 ; ++i)
                c[i] = p[i]*255.f;

            setPixel(vec3::construct((float)color / (1<<24)),uivec2(i,j));
        }
    }

    return *this;
}

bool Image::exportBmp(const std::string& str) const
{
    std::ofstream stream(str, std::ios_base::binary);
    if(!stream)
        return false;


    ubyte file[14] = {
        'B','M', // magic
        0,0,0,0, // size in bytes
        0,0, // app data
        0,0, // app data
        40+14,0,0,0 // start of data offset
    };
    ubyte info[40] = {
        40,0,0,0, // info hd size
        0,0,0,0, // width
        0,0,0,0, // heigth
        1,0, // number color planes
        24,0, // bits per pixel
        0,0,0,0, // compression is none
        0,0,0,0, // image bits size
        0x13,0x0B,0,0, // horz resoluition in pixel / m
        0x13,0x0B,0,0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
        0,0,0,0, // #colors in pallete
        0,0,0,0, // #important colors
        };

    int w=_size.x();
    int h=_size.y();

    int padSize  = (4-w%4)%4;
    int sizeData = w*h*3 + h*padSize;
    int sizeAll  = sizeData + sizeof(file) + sizeof(info);

    file[ 2] = (ubyte)( sizeAll    );
    file[ 3] = (ubyte)( sizeAll>> 8);
    file[ 4] = (ubyte)( sizeAll>>16);
    file[ 5] = (ubyte)( sizeAll>>24);

    info[ 4] = (ubyte)( w   );
    info[ 5] = (ubyte)( w>> 8);
    info[ 6] = (ubyte)( w>>16);
    info[ 7] = (ubyte)( w>>24);

    info[ 8] = (ubyte)( h    );
    info[ 9] = (ubyte)( h>> 8);
    info[10] = (ubyte)( h>>16);
    info[11] = (ubyte)( h>>24);

    info[24] = (ubyte)( sizeData    );
    info[25] = (ubyte)( sizeData>> 8);
    info[26] = (ubyte)( sizeData>>16);
    info[27] = (ubyte)( sizeData>>24);

    stream.write( (char*)file, sizeof(file) );
    stream.write( (char*)info, sizeof(info) );

    ubyte pad[3] = {0,0,0};

    for (int x=0 ; x<w ; ++x)
    {
        for (int y=0 ; y<h ; ++y)
        {
            char* b=reinterpret_cast<char*>(&_data[x*_size.y()+y]);
            stream.write(b+2, 1);
            stream.write(b+1, 1);
            stream.write(b, 1);
        }
        stream.write((char*)pad, padSize);
    }

    return true;
}

Image& Image::buildFromData(unsigned char* data, const TextureLoader::ImageFormat& format)
{
    build(format.size);
    for(uint x=0 ; x<format.size.x() ; ++x)
        for(uint y=0 ; y<format.size.y() ; ++y)
    {
        vec4 pix;
        for(uint i=0 ; i<format.nbComponent ; ++i)
            pix[i] = data[(x*format.size.y()+y)*format.nbComponent+i]/255.f;
        setPixel(pix, {x,y});
    }

    return *this;
}

Image& Image::transpose()
{
    uint* cpy_data = _data;
    _data = nullptr;
    build({_size.y(), _size.x()});

    for(size_t i=0 ; i<_size.y() ; ++i)
        for(size_t j=0 ; j<_size.x() ; ++j)
    {
        _data[j*_size.y()+i] = cpy_data[i*_size.x()+j];
    }
    delete[] cpy_data;

    return *this;
}

Image& Image::transform(const imat2& m)
{
    if(_size.x() != _size.y())
        return *this;

    uint* cpy_data = _data;
    _data = nullptr;
    build({_size.y(), _size.x()});
    ivec2 v;
    for(size_t i=0 ; i<_size.x() ; ++i)
        for(size_t j=0 ; j<_size.y() ; ++j)
    {
        if(_size.x() % 2 == 1)
            v = ivec2(i,j) - ivec2((_size.x()-1)/2, (_size.y()-1)/2);
        else
            v = ivec2( i<_size.x()/2 ? i-_size.x()/2 : i-_size.x()/2+1 ,
                       j<_size.y()/2 ? j-_size.y()/2 : j-_size.y()/2+1);

        v = m*v;

        if(_size.x() % 2 == 1)
            v = v + ivec2((_size.x()-1)/2, (_size.y()-1)/2);
        else
            v = ivec2( v.x()<0 ? v.x()+_size.x()/2 : v.x()+_size.x()/2-1 ,
                       v.y()<0 ? v.y()+_size.y()/2 : v.y()+_size.y()/2-1);

        _data[v.x()*_size.y()+v.y()] = cpy_data[i*_size.y()+j];
    }
    delete[] cpy_data;

    return *this;
}

}
}
