#include "Texture.h"
#include <fstream>

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

Texture::~Texture()
{
    uint id = _id;
    openGL.pushGLTask([=]()
    {
        glDeleteTextures(1, &id);
    });
}

void Texture::makeBindless() const
{
    if(_isBindless) return;

    _isBindless = true;
    _handle = glGetTextureHandleARB(_id);
    glMakeTextureHandleResidentARB(_handle);
}

void Texture::bind(uint index) const
{
    openGL.bindTexture(_id, toGLType(_type), index);
}

Texture* Texture::genTexture2D(const GenTexParam& param, const float* data, uint nbComponent)
{
    return genTexture2D(GL_FLOAT, param, data, nbComponent);
}

Texture* Texture::genTexture2D(const GenTexParam& param, const ubyte* data, uint nbComponent)
{
    return genTexture2D(GL_UNSIGNED_BYTE, param, data, nbComponent);
}

Texture* Texture::genTextureArray2D(const GenTexParam& param, const ubyte* data, uint nbC)
{
    return genTextureArray2D(GL_UNSIGNED_BYTE, param, data, nbC);
}

Texture* Texture::genTextureArray2D(const GenTexParam& param, const float* data, uint nbC)
{
    return genTextureArray2D(GL_FLOAT, param, data, nbC);
}

static inline uint glDataFormat(int nbComponent)
{
    switch(nbComponent)
    {
        case 1: return GL_RED;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: return GL_NONE;
    }
}

Texture* Texture::genTexture2D(uint dataType, const GenTexParam& param, const void* data, uint nbComponent)
{
    uint idTex;
    glGenTextures(1, &idTex);
    openGL.bindTexture(idTex, GL_TEXTURE_2D, 0);

    int level = param.nbLevels;
    if(level <= 0)
        level = 1+log2_ui(ge_power2(std::max(param.size.x(), param.size.y())));

    glTexStorage2D(GL_TEXTURE_2D, level, toGLFormat(param.format), param.size.x(), param.size.y());

    uint dataFormat = glDataFormat(nbComponent);
    if(data && dataFormat != GL_NONE)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, param.size.x(), param.size.y(), dataFormat, dataType, data);
        if(level > 1)
            glGenerateMipmap(GL_TEXTURE_2D);    
    }

    Texture::setupParameter(GL_TEXTURE_2D, param.repeat, param.linear, param.trilinear, param.depthMode, param.anisotropy);

    Texture* tex = new Texture;
    tex->_format = param.format;
    tex->_id = idTex;
    tex->_size = param.size;
    tex->_type = TEXTURE_2D;
    return tex;
}

Texture* Texture::genTextureCube(const GenTexParam& param, const vector<ubyte*>& data, uint nbComponent)
{
    uint idTex;
    glGenTextures(1, &idTex);
    openGL.bindTexture(idTex, GL_TEXTURE_CUBE_MAP, 0);

    int level = param.nbLevels;
    if(level <= 0)
        level = 1+log2_ui(ge_power2(std::max(param.size.x(), param.size.y())));

    glTexStorage2D(GL_TEXTURE_CUBE_MAP, level, toGLFormat(param.format), param.size.x(), param.size.y());

    uint dataFormat = glDataFormat(nbComponent);
    if(data.size()>= 6 && dataFormat != GL_NONE)
    {
        if(data[0]) glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, param.size.x(), param.size.y(), dataFormat, GL_UNSIGNED_BYTE, data[0]);
        if(data[1]) glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 0, 0, param.size.x(), param.size.y(), dataFormat, GL_UNSIGNED_BYTE, data[1]);
        if(data[2]) glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0, 0, param.size.x(), param.size.y(), dataFormat, GL_UNSIGNED_BYTE, data[2]);
        if(data[3]) glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0, 0, param.size.x(), param.size.y(), dataFormat, GL_UNSIGNED_BYTE, data[3]);
        if(data[4]) glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, 0, 0, param.size.x(), param.size.y(), dataFormat, GL_UNSIGNED_BYTE, data[4]);
        if(data[5]) glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, 0, 0, param.size.x(), param.size.y(), dataFormat, GL_UNSIGNED_BYTE, data[5]);

        if(level > 1)
            glGenerateMipmap(GL_TEXTURE_2D);
    }

    Texture* tex = new Texture;
    tex->_format = param.format;
    tex->_id = idTex;
    tex->_size = param.size;
    tex->_type = CUBE_MAP;
    return tex;
}

Texture* Texture::genTextureArray2D(uint dataType, const GenTexParam& param, const void* data, uint nbComponent)
{
    uint idTex;
    glGenTextures(1, &idTex);
    openGL.bindTexture(idTex, GL_TEXTURE_2D_ARRAY, 0);

    int level = param.nbLevels;
    if(level <= 0)
        level = 1+log2_ui(ge_power2(std::max(param.size.x(), param.size.y())));

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, level, toGLFormat(param.format), param.size.x(), param.size.y(), param.size.z());

    uint dataFormat = glDataFormat(nbComponent);
    if(data && dataFormat != GL_NONE)
    {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, param.size.x(), param.size.y(), param.size.z(), dataFormat, dataType, data);
        if(level > 1)
            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    Texture* tex = new Texture;
    tex->_format = param.format;
    tex->_id = idTex;
    tex->_size = param.size;
    tex->_type = ARRAY_2D;
    return tex;
}

uint Texture::genTextureSampler(bool repeat, bool linear, bool mipmapLinear, bool depthTest, int anisotropy)
{
    uint id=0;
    glGenSamplers(1, &id);

    if(linear && mipmapLinear)
        glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    else if(linear && !mipmapLinear)
        glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    else if(!linear && mipmapLinear)
        glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    else
        glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    if(repeat)
    {
        glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else
    {
        glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    if(linear)
        glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    else
        glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    if(openGL.hardward(GLState::Hardward::ANISOTROPY) && anisotropy > 0)
        glSamplerParameteri(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

    if(depthTest)
    {
        //glSamplerParameteri(id, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
        glSamplerParameteri(id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glSamplerParameteri(id, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }

    return  id;
}

void Texture::setupParameter(GLenum type, bool repeat, bool linear, bool mipmapLinear,bool depthTest, int anisotropy)
{
    if(linear && mipmapLinear)
        glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    else if(linear && !mipmapLinear)
        glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    else if(!linear && mipmapLinear)
        glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    else
        glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    if(repeat)
    {
        glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else
    {
        glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    if(linear)
        glTexParameteri(type, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    else
        glTexParameteri(type, GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    if(openGL.hardward(GLState::Hardward::ANISOTROPY) && anisotropy > 0)
        glTexParameteri(type, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

    if(depthTest)
    {
        //glSamplerParameteri(id, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
        glTexParameteri(type, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(type, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
}

void Texture::removeTextureSampler(uint sampler)
{
    glDeleteSamplers(1, &sampler);
}

template<class T>
void __write(std::ostream& stream, const T& data)
{
    stream.write(reinterpret_cast<const char*>(&data), sizeof(data));
}

void Texture::exportTexture(Texture* tex, std::string filename, int nbMipmap)
{
    std::ofstream fs(filename, std::ios_base::binary);
    if(!fs || !tex || tex->size() == uivec3(0,0,0) || tex->format()==GL_RGB10_A2 || tex->format()==GL_R3_G3_B2 || tex->format()==GL_R11F_G11F_B10F)
    {
        LOG("Unuable to export ", filename);
        return;
    }

    int effectiveMipMap = nbMipmap;
    if(nbMipmap <= 0)
        effectiveMipMap = 1;

    char header[4] = {93,67,34,12};
    fs.write(header,4);

    __write(fs, tex->_size);
    __write(fs, tex->_format);
    __write(fs, tex->_type);
    __write(fs, nbMipmap);

    ubyte* dat;

    tex->bind(0);
    switch(tex->type())
    {
    case TEXTURE_2D:
        dat = new ubyte[tex->_size.x() * tex->_size.y() * bytePerPixel(tex->_format)];
        for(int i=0 ; i<nbMipmap ; ++i)
        {
            glGetTexImage(GL_TEXTURE_2D, i, toExternalFormat(tex->_format), isFloatFormat(tex->_format) ? GL_FLOAT : GL_UNSIGNED_BYTE, dat);
            fs.write(reinterpret_cast<char*>(dat), (tex->_size.x() >> i) * (tex->_size.y() >> i) * bytePerPixel(tex->_format));
        }
        delete[] dat;
        break;

    case CUBE_MAP:
        dat = new ubyte[tex->_size.x() * tex->_size.y() * bytePerPixel(tex->_format)];
        for(int i=0 ; i<effectiveMipMap ; ++i)
        {
            glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X, i, toExternalFormat(tex->_format), isFloatFormat(tex->_format) ? GL_FLOAT : GL_UNSIGNED_BYTE, dat);
            fs.write(reinterpret_cast<char*>(dat), (tex->_size.x() >> i) * (tex->_size.y() >> i) * bytePerPixel(tex->_format));

            glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, i, toExternalFormat(tex->_format), isFloatFormat(tex->_format) ? GL_FLOAT : GL_UNSIGNED_BYTE, dat);
            fs.write(reinterpret_cast<char*>(dat), (tex->_size.x() >> i) * (tex->_size.y() >> i) * bytePerPixel(tex->_format));

            glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, i, toExternalFormat(tex->_format), isFloatFormat(tex->_format) ? GL_FLOAT : GL_UNSIGNED_BYTE, dat);
            fs.write(reinterpret_cast<char*>(dat), (tex->_size.x() >> i) * (tex->_size.y() >> i) * bytePerPixel(tex->_format));

            glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, i, toExternalFormat(tex->_format), isFloatFormat(tex->_format) ? GL_FLOAT : GL_UNSIGNED_BYTE, dat);
            fs.write(reinterpret_cast<char*>(dat), (tex->_size.x() >> i) * (tex->_size.y() >> i) * bytePerPixel(tex->_format));

            glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, i, toExternalFormat(tex->_format), isFloatFormat(tex->_format) ? GL_FLOAT : GL_UNSIGNED_BYTE, dat);
            fs.write(reinterpret_cast<char*>(dat), (tex->_size.x() >> i) * (tex->_size.y() >> i) * bytePerPixel(tex->_format));

            glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, i, toExternalFormat(tex->_format), isFloatFormat(tex->_format) ? GL_FLOAT : GL_UNSIGNED_BYTE, dat);
            fs.write(reinterpret_cast<char*>(dat), (tex->_size.x() >> i) * (tex->_size.y() >> i) * bytePerPixel(tex->_format));
        }
        delete[] dat;
        break;

    default:
        LOG("Unsuported texture type for export (", filename, ")");
        return;
    }
}

Texture* Texture::genTextureFromRawData(ubyte* rawData, GenTexParam param)
{
    if(rawData == nullptr)
        return nullptr;

    char header[4] = {93,67,34,12};
    if(rawData[0] != header[0] || rawData[1] != header[1] || rawData[2] != header[2] || rawData[3] != header[3])
        return nullptr;

    rawData += 4;
    uivec3 size = *reinterpret_cast<uivec3*>(rawData); rawData += sizeof(uivec3);
    Format format = *reinterpret_cast<Format*>(rawData); rawData += sizeof(Format);
    Type type = *reinterpret_cast<Type*>(rawData); rawData += sizeof(Type);
    int mipMap = *reinterpret_cast<int*>(rawData); rawData += sizeof(int);

    int effectiveMipmap = mipMap;
    if(mipMap <= 0)
        effectiveMipmap = 1;

    Texture* tex = nullptr;
    param.size = size;
    param.nbLevels = mipMap;
    param.format = format;

    switch(type)
    {
    case TEXTURE_2D:
        tex = genTexture2D(param);
        break;
    case CUBE_MAP:
        tex = genTextureCube(param);
        break;
    default: break;
    }

    if(tex == nullptr)
        return nullptr;

    tex->bind(0);
    for(int i=0 ; i<effectiveMipmap ; ++i)
    {
        switch(type)
        {
        case TEXTURE_2D:
            glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, size.x() >> i, size.y() >> i,
                            toExternalFormat(format), isFloatFormat(format) ? GL_FLOAT : GL_UNSIGNED_BYTE, rawData);
            rawData += (size.x() >> i) * (size.y() >> i) * bytePerPixel(format);
            break;
        case CUBE_MAP:
            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, i, 0, 0,  size.x() >> i, size.y() >> i,
                            toExternalFormat(format), isFloatFormat(format) ? GL_FLOAT : GL_UNSIGNED_BYTE, rawData);
            rawData += (size.x() >> i) * (size.y() >> i) * bytePerPixel(format);

            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, i, 0, 0,  size.x() >> i, size.y() >> i,
                            toExternalFormat(format), isFloatFormat(format) ? GL_FLOAT : GL_UNSIGNED_BYTE, rawData);
            rawData += (size.x() >> i) * (size.y() >> i) * bytePerPixel(format);

            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, i, 0, 0,  size.x() >> i, size.y() >> i,
                            toExternalFormat(format), isFloatFormat(format) ? GL_FLOAT : GL_UNSIGNED_BYTE, rawData);
            rawData += (size.x() >> i) * (size.y() >> i) * bytePerPixel(format);

            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, i, 0, 0,  size.x() >> i, size.y() >> i,
                            toExternalFormat(format), isFloatFormat(format) ? GL_FLOAT : GL_UNSIGNED_BYTE, rawData);
            rawData += (size.x() >> i) * (size.y() >> i) * bytePerPixel(format);

            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, i, 0, 0,  size.x() >> i, size.y() >> i,
                            toExternalFormat(format), isFloatFormat(format) ? GL_FLOAT : GL_UNSIGNED_BYTE, rawData);
            rawData += (size.x() >> i) * (size.y() >> i) * bytePerPixel(format);

            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, i, 0, 0,  size.x() >> i, size.y() >> i,
                            toExternalFormat(format), isFloatFormat(format) ? GL_FLOAT : GL_UNSIGNED_BYTE, rawData);
            rawData += (size.x() >> i) * (size.y() >> i) * bytePerPixel(format);
            break;

        default: break;
        }
    }

    if(mipMap <= 0)
        glGenerateMipmap(toGLType(type));

    return tex;
}

}
}
