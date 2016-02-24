#ifndef GLSTATE_H_INCLUDED
#define GLSTATE_H_INCLUDED

#include "GL/glew.h"

#include <queue>

#include "core/core.h"
#include "core/Singleton.h"

#define BUFFER_OFFSET(a) ((char*)NULL + (a))

#define GL_ASSERT() tim::renderer::GLState::assertGLError(__FILE__,__LINE__)

#include "core/MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    static const size_t MAX_TEXTURE_UNIT=32;
    static const size_t MAX_IMAGE_UNIT=16;
    static const size_t MAX_BUFFER_ATTACHEMENT = 8;

    class GLState : public Singleton<GLState>, boost::noncopyable
    {
        friend class Singleton<GLState>;

    public:
        void resetStates();
        void applyAll();

        static bool assertGLError(const char*, size_t);
        static std::string glErrorToString(GLenum);

        static void clearDepth();
        void clearColor(const vec4&);
        static void flush();
        static void finish();

        bool bindVertexBuffer(uint);
        bool bindVao(uint);
        bool bindPixelBufferUnpack(uint);
        bool bindElementArrayBuffer(uint);
        bool bindShaderStorageBuffer(uint, uint index=0);
        bool bindUniformBuffer(uint, uint index=0);
        bool bindShader(uint);
        bool bindFrameBuffer(uint);
        bool bindTexture(uint, GLenum, uint);
        bool bindTextureSampler(uint, uint);

        void bindImageTexture(uint id, uint index, GLenum, GLenum);

        void unbindVertexBuffer(uint);
        void unbindPixelBufferUnpack(uint);
        void unbindShaderStorageBuffer(uint);
        void unbindElementArrayBuffer(uint);
        void unbindUniformBuffer(uint);
        void unbindShader(uint);
        void unbindFrameBuffer(uint);
        void unbindTexture(uint, GLenum, uint);
        void unbindTextureSampler(uint, uint);

        void setViewPort(const uivec2&, const uivec2&);
        void depthTest(bool);
        void depthFunc(uint);
        void depthMask(bool);
        void colorMask(const Vector4<bool>&);
        void cullFace(bool);
        void cullFaceMode(uint);
        void blend(bool);
        void blendFunc(const Vector2<uint>&);
        void blendEquation(uint);
        void alphaFunc(uint, float);
        void alphaTest(bool);
        void scissorTest(bool);
        void scissorParam(const uivec2&, const uivec2&);
        void logicColor(bool);
        void logicOp(uint);

        enum Hardward
        {
            TEXTURE2D_SIZE,
            TEXTURE3D_SIZE,
            CUBEMAP_SIZE,
            FRAGMENT_TEX_UNITS,
            VERTEX_TEX_UNITS,
            GEOMETRY_TEX_UNITS,
            COMBINED_TEX_UNITS,

            MAX_VBO_SIZE,
            MAX_IBO_SIZE,
            MAX_UNIFORM_BLOCK_SIZE,

            FRAGMENT_OUT,
            FRAGMENT_IN,
            FRAGMENT_VEC_UNIFORM,

            GEOMETRY_IN,
            GEOMETRY_OUT,
            GEOMETRY_UNIFORM,
            GEOMETRY_OUT_VERTICES,

            VERTEX_OUT,
            VERTEX_ATTRIBS,
            VERTEX_VEC_UNIFORM,

            MAJOR_VERSION,
            MINOR_VERSION,

            ANISOTROPY,
            ANISOTROPY_SAMPLES,

            MAX_SHADER_STORAGE_SIZE,

            MAX_COMPUTE_WORK_GROUP_COUNT,
            MAX_COMPUTE_WORK_GROUP_SIZE,
            MAX_COMPUTE_WORK_GROUP_INVOCATION,

            LAST,
        };

        int hardward(Hardward hp) const;
        void getHardwardProperties();
        std::string strHardward() const;

        void pushGLTask(const std::function<void()>&);

        void execAllGLTask();
        size_t execOneGLTask();

        template <class F>
        void execGLTaskWhile(const F&);

    protected:
        GLState();
        virtual ~GLState() = default;

    private:

        /* Hardware propertys */
        int _hardwardProperties[LAST] = {0};

        /* State */
        vec4 _clearColor;
        uint _enabledTexture[MAX_TEXTURE_UNIT];
        uint _typeEnabledTexture[MAX_TEXTURE_UNIT];
        uint _samplerTexture[MAX_TEXTURE_UNIT];
        uint _textureUnit;
        uivec2 _viewPort[2];
        Vector2<uint> _blendFunc;
        float _alphaThreshold;
        Vector4<bool> _colorMask;
        uivec2 _scissorCoord, _scissorSize;

        uint _uboBinded[MAX_BUFFER_ATTACHEMENT];
        uint _ssboBinded[MAX_BUFFER_ATTACHEMENT];

        enum
        {
            ARRAY_BUFFER=0,
            VAO,
            ELEMENT_ARRAY_BUFFER,
            PIXEL_BUFFER_UNPACK,
            SHADER,
            FRAME_BUFFER,
            BLEND_EQUATION,
            CULL_FACE_MODE,
            ALPHA_FUNC,
            DEPTH_FUNC,
            OPCODE,
            NB_STATES,
        };
        uint _glStates[NB_STATES];

        enum
        {
            DEPTH_TEST=0,
            DEPTH_MASK,
            BLEND,
            CULL_FACE,
            ALPHA_TEST,
            SCISSOR_TEST,
            LOGIC_COLOR,
            NB_BSTATES,
        };
        bool _glBoolStates[NB_BSTATES];

        static void glSet(uint, bool);

        boost::mutex _glTaskAccess;
        std::queue<std::function<void()>> _glTask;
    };

    extern GLState& openGL;

    inline void GLState::pushGLTask(const std::function<void()> & f)
    {
        boost::lock_guard<boost::mutex> guard(_glTaskAccess);
        _glTask.push(f);
    }

    inline void GLState::execAllGLTask()
    {
        boost::lock_guard<boost::mutex> guard(_glTaskAccess);
        while(!_glTask.empty())
        {
            _glTask.front()();
            _glTask.pop();
        }
    }

    inline size_t GLState::execOneGLTask()
    {
        boost::lock_guard<boost::mutex> guard(_glTaskAccess);
        if(_glTask.empty())
            return _glTask.size();

        _glTask.front()();
        _glTask.pop();

        return _glTask.size();
    }

    template <class F>
    void GLState::execGLTaskWhile(const F& f)
    {
        boost::lock_guard<boost::mutex> guard(_glTaskAccess);
        while(!_glTask.empty() && f())
        {
            _glTask.front()();
            _glTask.pop();
        }
    }

    inline void GLState::glSet(uint s, bool b)
    {
        if(b) glEnable(s);
        else glDisable(s);
    }

    inline int GLState::hardward(Hardward hp) const
    {
        return _hardwardProperties[hp];
    }

    inline void GLState::bindImageTexture(uint id, uint index, GLenum access, GLenum format)
    {
        glBindImageTexture(index, id, 0, GL_FALSE, 0, access, format);
    }

    inline bool GLState::bindVertexBuffer(uint id)
    {
        if(_glStates[ARRAY_BUFFER] != id)
        {
            _glStates[ARRAY_BUFFER] = id;
            glBindBuffer(GL_ARRAY_BUFFER, id);
            return true;
        }
        return false;
    }

    inline bool GLState::bindVao(uint id)
    {
        if(_glStates[VAO] != id)
        {
            _glStates[VAO] = id;
            glBindVertexArray(id);
            _glStates[ELEMENT_ARRAY_BUFFER] = 0;
            _glStates[ARRAY_BUFFER] = 0;
            return true;
        }
        return false;
    }

    inline bool GLState::bindElementArrayBuffer(uint id)
    {
        if(_glStates[ELEMENT_ARRAY_BUFFER] != id)
        {
            _glStates[ELEMENT_ARRAY_BUFFER] = id;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
            return true;
        }
        return false;
    }

    inline bool GLState::bindPixelBufferUnpack(uint id)
    {
        if(_glStates[PIXEL_BUFFER_UNPACK] != id)
        {
            _glStates[PIXEL_BUFFER_UNPACK] = id;
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, id);
            return true;
        }
        return false;
    }

    inline bool GLState::bindShaderStorageBuffer(uint id, uint index)
    {
        if(_ssboBinded[index] != id)
        {
            _ssboBinded[index] = id;
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, id);
            return true;
        }
        return false;
    }


    inline bool GLState::bindShader(uint id)
    {
        if(_glStates[SHADER] != id)
        {
            _glStates[SHADER] = id;
            glUseProgram(id);
            return true;
        }
        return false;
    }

    inline bool GLState::bindTexture(uint id, GLenum type, uint unit)
    {
        if(_textureUnit != unit)
        {
            _textureUnit=unit;
            glActiveTexture(GL_TEXTURE0+_textureUnit);
        }

        if(_enabledTexture[unit] != id)
        {
            _enabledTexture[unit] = id;
            _typeEnabledTexture[unit] = type;
            glBindTexture(type, id);
            return true;
        }
        return false;
    }

    inline bool  GLState::bindTextureSampler(uint sampler, uint unit)
    {
        if(_samplerTexture[unit] != sampler)
        {
            _samplerTexture[unit]=sampler;
            glBindSampler(unit, sampler);
            return true;
        }
        return false;
    }

    inline bool GLState::bindFrameBuffer(uint id)
    {
        if(_glStates[FRAME_BUFFER] != id)
        {
            _glStates[FRAME_BUFFER]=id;
            glBindFramebuffer(GL_FRAMEBUFFER, id);
            return true;
        }
        return false;
    }

    inline bool GLState::bindUniformBuffer(uint id, uint index)
    {
        if(_uboBinded[index] != id)
        {
            _uboBinded[index]=id;
            glBindBufferBase(GL_UNIFORM_BUFFER, index, id);
            return true;
        }
        return false;
    }

    inline void GLState::unbindVertexBuffer(uint id)
    {
        if(_glStates[ARRAY_BUFFER] == id)
        {
            _glStates[ARRAY_BUFFER] = 0;
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }

    inline void GLState::unbindShaderStorageBuffer(uint id)
    {
        for(uint index=0 ; index < MAX_BUFFER_ATTACHEMENT ; ++index)
            if(_ssboBinded[index] == id)
            {
                _ssboBinded[index] = 0;
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, 0);
            }
    }

    inline void GLState::unbindUniformBuffer(uint id)
    {
        for(uint index=0 ; index < MAX_BUFFER_ATTACHEMENT ; ++index)
            if(_uboBinded[index] == id)
            {
                _uboBinded[index]=0;
                glBindBufferBase(GL_UNIFORM_BUFFER, index, 0);
            }
    }

    inline void GLState::unbindPixelBufferUnpack(uint id)
    {
        if(_glStates[PIXEL_BUFFER_UNPACK] == id)
        {
            _glStates[PIXEL_BUFFER_UNPACK] = 0;
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
    }

    inline void GLState::unbindElementArrayBuffer(uint id)
    {
        if(_glStates[ELEMENT_ARRAY_BUFFER] == id)
        {
            _glStates[ELEMENT_ARRAY_BUFFER] = 0;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }

    inline void GLState::unbindShader(uint id)
    {
        if(_glStates[SHADER] == id)
        {
            _glStates[SHADER] = 0;
            glUseProgram(0);
        }
    }

    inline void GLState::unbindFrameBuffer(uint id)
    {
        if(_glStates[FRAME_BUFFER] == id)
        {
            _glStates[FRAME_BUFFER] = 0;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    inline void GLState::unbindTexture(uint id, GLenum type, uint unit)
    {
        if(_textureUnit != unit)
        {
            _textureUnit=unit;
            glActiveTexture(GL_TEXTURE0+_textureUnit);
        }

        if(_enabledTexture[unit] == id)
        {
            _enabledTexture[unit] = 0;
            _typeEnabledTexture[unit] = type;
            glBindTexture(type, 0);
        }
    }

    inline void GLState::unbindTextureSampler(uint sampler, uint unit)
    {
        if(_samplerTexture[unit] == sampler)
        {
            _samplerTexture[unit]=0;
            glBindSampler(unit, 0);
        }
    }

    inline void GLState::setViewPort(const uivec2& coord, const uivec2& size)
    {
        if(coord != _viewPort[0] || size != _viewPort[1])
        {
            _viewPort[0]=coord;
            _viewPort[1]=size;
            glViewport(coord.x(), coord.y(), size.x(), size.y());
        }
    }

    inline void GLState::depthTest(bool b)
    {
        if(b != _glBoolStates[DEPTH_TEST])
        {
            _glBoolStates[DEPTH_TEST]=b;
            glSet(GL_DEPTH_TEST, b);
        }
    }

    inline void GLState::depthMask(bool b)
    {
        if(b != _glBoolStates[DEPTH_MASK])
        {
            _glBoolStates[DEPTH_MASK]=b;
            if(b) glDepthMask(GL_TRUE);
            else glDepthMask(GL_FALSE);
        }
    }

    inline void GLState::blend(bool b)
    {
        if(b != _glBoolStates[BLEND])
        {
            _glBoolStates[BLEND]=b;
            //glSet(GL_BLEND, b);
            if(b) glEnablei(GL_BLEND, 0);
            else glDisable(GL_BLEND);
        }
    }

    inline void GLState::alphaTest(bool b)
    {
        if(b != _glBoolStates[ALPHA_TEST])
        {
            _glBoolStates[ALPHA_TEST]=b;
            glSet(GL_ALPHA_TEST, b);
        }
    }

    inline void GLState::cullFace(bool b)
    {
        if(b != _glBoolStates[CULL_FACE])
        {
            _glBoolStates[CULL_FACE]=b;
            glSet(GL_CULL_FACE, b);
        }
    }

    inline void GLState::blendFunc(const Vector2<uint>& b)
    {
        if(b != _blendFunc)
        {
            _blendFunc=b;
            glBlendFunci(0, b.x(), b.y());
        }
    }

    inline void GLState::blendEquation(uint e)
    {
        if(e != _glStates[BLEND_EQUATION])
        {
            _glStates[BLEND_EQUATION]=e;
            glBlendEquationi(0, e);
        }
    }

    inline void GLState::cullFaceMode(uint m)
    {
        if(m != _glStates[CULL_FACE_MODE])
        {
            _glStates[CULL_FACE_MODE]=m;
            glCullFace(m);
        }
    }

    inline void GLState::alphaFunc(uint func, float threshold)
    {
        if(func != _glStates[ALPHA_FUNC] || _alphaThreshold!=threshold)
        {
            _glStates[ALPHA_FUNC]=func;
            _alphaThreshold=threshold;
            glAlphaFunc(func, threshold);
        }
    }

    inline void GLState::depthFunc(uint func)
    {
        if(func != _glStates[DEPTH_FUNC])
        {
            _glStates[DEPTH_FUNC]=func;
            glDepthFunc(func);
        }
    }

    inline void GLState::colorMask(const Vector4<bool>& m)
    {
        if(_colorMask!=m)
        {
            _colorMask=m;
            glColorMask(m[0], m[1], m[2], m[3]);
        }
    }

    inline void GLState::scissorTest(bool b)
    {
        if(_glBoolStates[SCISSOR_TEST]!=b)
        {
            _glBoolStates[SCISSOR_TEST]=b;
            glSet(GL_SCISSOR_TEST, b);
        }
    }

    inline void GLState::logicColor(bool b)
    {
        if(_glBoolStates[LOGIC_COLOR]!=b)
        {
            _glBoolStates[LOGIC_COLOR]=b;
            glSet(GL_COLOR_LOGIC_OP, b);
        }
    }

    inline void GLState::scissorParam(const uivec2& coord, const uivec2& size)
    {
        if(_scissorCoord!=coord || _scissorSize!=size)
        {
            _scissorCoord=coord;
            _scissorSize=size;
            glScissor(coord.x(), coord.y(), size.x(), size.y());
        }
    }

    inline void GLState::logicOp(uint opcode)
    {
        if(_glStates[OPCODE]!=opcode)
        {
            _glStates[OPCODE]=opcode;
            glLogicOp(opcode);
        }
    }

    inline void GLState::clearDepth()
    {
        openGL.depthMask(true);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    inline void GLState::clearColor(const vec4& col)
    {
        if(_clearColor != col)
        {
            _clearColor=col;
            glClearColor(col.x(),col.y(),col.z(), col.w());
        }
        colorMask({true,true,true,true});
        glClear(GL_COLOR_BUFFER_BIT);
    }

    inline void GLState::flush()
    {
        glFlush();
    }

    inline void GLState::finish()
    {
        glFinish();
    }
}
}
#include "MemoryLoggerOff.h"

#endif // GLSTATE_H_INCLUDED
