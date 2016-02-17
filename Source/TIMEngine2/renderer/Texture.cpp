#include "Texture.h"

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
        level = 1+log2_ui(upper_power_2(std::max(param.size.x(), param.size.y())));

    glTexStorage2D(GL_TEXTURE_2D, level, toGLFormat(param.format), param.size.x(), param.size.y());

    uint dataFormat = glDataFormat(nbComponent);
    if(data && dataFormat != GL_NONE)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, param.size.x(), param.size.y(), dataFormat, dataType, data);
        if(level > 1)
            glGenerateMipmap(GL_TEXTURE_2D);
    }

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
        level = 1+log2_ui(upper_power_2(std::max(param.size.x(), param.size.y())));

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
        glSamplerParameteri(id, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
        glSamplerParameteri(id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glSamplerParameteri(id, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }

    return  id;
}

void Texture::removeTextureSampler(uint sampler)
{
    glDeleteSamplers(1, &sampler);
}

}
}
