#include "TextureBufferPool.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

int TextureBufferPool::_idGenerator = 1;

TextureBufferPool::TextureBufferPool()
{

}

TextureBufferPool::~TextureBufferPool()
{
    for(auto p : _pool)
    {
        for(size_t i=0 ; i<p.second.size() ; ++i)
            p.second[i].clear();
    }
}

bool TextureBufferPool::Key::operator<(const Key& k) const
{
    if(res != k.res) return res < k.res;
    else if(type != k.type) return type < k.type;
    else if(hdr != k.hdr) return hdr < k.hdr;
    else return false;
}

void TextureBufferPool::InternBuffer::clear()
{
    delete buffer.fbo;
    for(auto ptr : buffer.texs)
        delete ptr;
}

TextureBufferPool::Buffer TextureBufferPool::getBuffer(const Key& k)
{
    vector<InternBuffer>& buf = _pool[k];

    for(size_t i=0 ; i<buf.size() ; ++i)
    {
        if(!buf[i].used)
        {
            buf[i].used = true;
            return buf[i].buffer;
        }
    }

    buf.push_back({createBuffer(k), true});
    _idToKey[_idGenerator] = k;
    _idGenerator++;

    return buf.back().buffer;
}

void TextureBufferPool::releaseBuffer(int id)
{
    auto it = _idToKey.find(id);
    if(it == _idToKey.end())
        return;

    vector<InternBuffer>& buf = _pool[it->second];
    for(size_t i=0 ; i<buf.size() ; ++i)
    {
        if(buf[i].buffer.id == id)
        {
            buf[i].used = false;
            return;
        }
    }
}

TextureBufferPool::Buffer TextureBufferPool::createBuffer(const Key& k)
{
    Buffer buf;
    if(k.type == Key::DEFERRED_BUFFER)
    {
        buf.fbo = new FrameBuffer(k.res.to<2>());
        FrameBuffer::setupDefferedFBO(*buf.fbo, buf.texs);
    }
    else if(k.type == Key::DEPTH_MAP_ARRAY)
    {
        renderer::Texture::GenTexParam p;
        p.format = renderer::Texture::Format::DEPTHCOMPONENT;
        p.nbLevels = 1;
        p.size = k.res;

        buf.texs.push_back(renderer::Texture::genTextureArray2D(p));
        buf.fbo = new FrameBuffer(k.res.to<2>());
    }
    else
    {
        if(!k.onlyTextures)
            buf.fbo = new FrameBuffer(k.res.to<2>());

        for(uint i=0 ; i<k.res.z() ; ++i)
        {
            Texture::GenTexParam param;
            param.size = uivec3(k.res.to<2>(),1);
            param.nbLevels = 1;
            param.format = k.hdr ? Texture::RGBA16F : Texture::RGBA8;

            buf.texs.push_back( Texture::genTexture2D(param) );

            if(!k.onlyTextures)
            {
                buf.fbo->attachTexture(i, buf.texs.back());
            }

        }

        if(!k.onlyTextures)
            buf.fbo->unbind();
    }

    buf.id = _idGenerator;
    return buf;
}

}
}

