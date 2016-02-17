#include "FrameBuffer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &_id);

    for(uint i=0 ; i<MAX_COLOR_ATTACHMENT ; ++i)
        _isTexAttached[i] = false;
}

FrameBuffer::FrameBuffer()
{
    glGenFramebuffers(1, &_id);
}

FrameBuffer::FrameBuffer(const uivec2& v) : FrameBuffer()
{
    _resolution = v;
}

FrameBuffer::FrameBuffer(const std::vector<Texture*>& colors, Texture* depth) : FrameBuffer()
{
    for(size_t i=0 ; i<colors.size() ; ++i)
        attachTexture(i, colors[i]);

    attachDepthTexture(depth);
}

void FrameBuffer::attachTexture(uint attachment, Texture* tex, uint level, uint layer)
{
    _isTexAttached[attachment] = (tex!=nullptr);

    if(!tex)  return;

    openGL.bindFrameBuffer(_id);

    switch(tex->type())
    {
    case Texture::TEXTURE_2D:
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, tex->id(), level); break;
    case Texture::ARRAY_2D:
        glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, tex->id(), level, layer); break;

    case Texture::CUBE_MAP:
        switch(layer)
        {
            case 0: glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X, tex->id(), level); break;
            case 1: glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, tex->id(), level); break;
            case 2: glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, tex->id(), level); break;
            case 3: glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, tex->id(), level); break;
            case 4: glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, tex->id(), level); break;
            case 5: glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, tex->id(), level); break;
        }
        break;

    default: break;
    }

    enableAllAttachment();
}

void FrameBuffer::attachDepthTexture(Texture* tex, uint layer)
{
    if(!tex) return;

    openGL.bindFrameBuffer(_id);

    switch(tex->type())
    {
    case Texture::TEXTURE_2D:
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex->id(), 0); break;
    case Texture::ARRAY_2D:
        glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex->id(), 0, layer); break;
    default: break;
    }
}

void FrameBuffer::bind() const
{
    openGL.bindFrameBuffer(_id);
    openGL.setViewPort(uivec2(), _resolution);
}

void FrameBuffer::unbind() const
{
    openGL.unbindFrameBuffer(_id);
}

void FrameBuffer::enableAllAttachment() const
{
    uint nbAttachment=0;
    GLenum attachment[MAX_COLOR_ATTACHMENT];

    for(uint i=0 ; i<MAX_COLOR_ATTACHMENT ; ++i)
    {
        if(_isTexAttached[i])
            attachment[nbAttachment++] = GL_COLOR_ATTACHMENT0 + i;
    }

    if(nbAttachment == 0)
        glDrawBuffer(GL_NONE);
    else
        glDrawBuffers(nbAttachment, attachment);
}

void FrameBuffer::setupDefferedFBO(FrameBuffer& fbo, vector<Texture*>& buffers)
{
    buffers.resize(4); // albedo normal material depth
    Texture::GenTexParam param;
    param.size = uivec3(fbo.resolution(),0);
    param.nbLevels = 1;

    param.format = Texture::Format::RGBA8;
    buffers[0] = Texture::genTexture2D(param);

    param.format = Texture::Format::RGB16F;
    buffers[1] = Texture::genTexture2D(param);

    param.format = Texture::Format::RGBA8;
    buffers[2] = Texture::genTexture2D(param);

    param.format = Texture::Format::DEPTHCOMPONENT;
    buffers[3] = Texture::genTexture2D(param);

    fbo.attachTexture(0, buffers[0]);
    fbo.attachTexture(1, buffers[1]);
    fbo.attachTexture(2, buffers[2]);
    fbo.attachDepthTexture(buffers[3]);
}

}
}
