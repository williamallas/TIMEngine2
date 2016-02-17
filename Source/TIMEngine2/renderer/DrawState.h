#ifndef DRAWSTATE_H_INCLUDED
#define DRAWSTATE_H_INCLUDED

#include "GLState.h"
#include "Shader.h"
#include <bitset>

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    struct DrawState
    {
    public:

        enum CompareFunc : uint
        {
            NEVER=0,
            NOTEQUAL,
            LESS,
            LEQUAL,
            GREATER,
            GEQUAL,
            EQUAL,
            ALWAYS,
        };

        enum BlendFunc : uint
        {
            ZERO=0,
            ONE,
            SRC_COLOR,
            ONE_MINUS_SRC_COLOR,
            DST_COLOR,
            ONE_MINUS_DST_COLOR,
            SRC_ALPHA,
            ONE_MINUS_SRC_ALPHA,
            DST_ALPHA,
            ONE_MINUS_DST_ALPHA,
            CONSTANT_COLOR,
            ONE_MINUS_CONSTANT_COLOR,
            CONSTANT_ALPHA,
            ONE_MINUS_CONSTANT_ALPHA,
        };

        enum BlendEquation : uint
        {
            ADD=0,
            SUB,
            REVERSE_SUB,
            MIN,
            MAX,
        };

        static uint toGLBlendFunc(BlendFunc);
        static uint toGLComparFunc(CompareFunc);
        static uint toGLBlendEquation(BlendEquation);

        DrawState()
        {
           setAlwaysFirst(false);
           setAlwaysLast(false);
           setBlend(false);
           setCullBackFace(true);
           setCullFace(true);
           setDepthTest(true);
           setWriteDepth(true);
           setDepthFunc(LESS);
           setBlendEquation(ADD);
           setBlendFunc({SRC_ALPHA, ONE_MINUS_SRC_ALPHA});
        }

        ~DrawState() = default;

        void setShader(Shader* shader) { _shader = shader; }

        void setAlwaysFirst(bool b)  { _states[Offset::AFIRST] = !b; }
        void setAlwaysLast(bool b)   { _states[Offset::ALAST] = b; }
        void setBlend(bool b)        { _states[Offset::BLEND] = b; }
        void setWriteDepth(bool b)   { _states[Offset::WDEPTH] = b; }
        void setDepthTest(bool b)    { _states[Offset::DEPTHT] = b; }

        void setCullFace(bool b)     { _states[Offset::CULLF] = b; }
        void setCullBackFace(bool b) { _states[Offset::CULLBF] = b; }

        void setDepthFunc(CompareFunc c)
        {
            for(uint i=0 ; i<3 ; ++i)
                _states[Offset::DEPTHF+i] = (c>>i)%2;
        }

        void setBlendEquation(BlendEquation be)
        {
            for(uint i=0 ; i<4 ; ++i)
                _states[Offset::BLENDE+i] = (be>>i)%2;
        }

        void setBlendFunc(const Vector2<BlendFunc>& funcs)
        {
            for(uint i=0 ; i<4 ; ++i) _states[Offset::BLENDF1+i] = (funcs[0]>>i)%2;
            for(uint i=0 ; i<4 ; ++i) _states[Offset::BLENDF2+i] = (funcs[1]>>i)%2;
        }

        Shader* shader() const { return _shader; }

        bool alwaysFirst()  const { return  !_states[Offset::AFIRST]; }
        bool alwaysLast()   const { return  _states[Offset::ALAST]; }
        bool blend()        const { return  _states[Offset::BLEND]; }
        bool writeDepth()   const { return  _states[Offset::WDEPTH]; }
        bool depthTest()    const { return  _states[Offset::DEPTHT]; }
        bool cullFace()     const { return  _states[Offset::CULLF]; }
        bool cullBackFace() const { return  _states[Offset::CULLBF]; }

        CompareFunc depthFunc() const
        {
            uint n=0;
            for(uint i=0 ; i<3;  ++i)
                n += _states[Offset::DEPTHF+i] << i;
            return static_cast<CompareFunc>(n);
        }

         BlendEquation blendEquation() const
        {
            uint n=0;
            for(uint i=0 ; i<4;  ++i)
                n += _states[Offset::BLENDE+i] << i;
            return static_cast<BlendEquation>(n);
        }

        Vector2<BlendFunc> blendFunc() const
        {
            uint n1=0;
            for(uint i=0 ; i<4;  ++i)
                n1 += _states[Offset::BLENDF1+i] << i;

            uint n2=0;
            for(uint i=0 ; i<4;  ++i)
                n2 += _states[Offset::BLENDF2+i] << i;

            return {static_cast<BlendFunc>(n1),static_cast<BlendFunc>(n2)};
        }

        bool operator<(const DrawState& state) const
        {
            if(_states != state._states) return _states.to_ulong() < state._states.to_ulong();
            else return _shader < state._shader;
        }

        void bind() const
        {
            _shader->bind();
            openGL.depthTest(depthTest());
            openGL.depthMask(writeDepth());
            openGL.depthFunc(toGLComparFunc(depthFunc()));
            openGL.cullFace(cullFace());
            openGL.cullFaceMode(cullBackFace() ? GL_BACK : GL_FRONT);
            openGL.blend(blend());
            openGL.blendEquation(toGLBlendEquation(blendEquation()));

            Vector2<BlendFunc> v = blendFunc();
            openGL.blendFunc({toGLBlendFunc(v[0]), toGLBlendFunc(v[1])});
        }

        void debug() {
         std::cout << "DepthTest: " << depthTest() << "\n"
                   << "WiteDepth: " << writeDepth() << "\n"
                   << "cullFace : " << cullFace() << "\n"
                   << "backFace : " << cullBackFace() << "\n"
                   << "blend    : " << blend() << "\n"
                   << "first    : " << alwaysFirst() << "\n"
                   << "last     : " << alwaysLast() << "\n"
                   << "binary: " << _states.to_string('0') << "\n";
        }

    private:
        Shader* _shader = nullptr;
        std::bitset<22> _states;

        struct Offset { enum : uint {
                AFIRST=21, ALAST=20, BLEND=19, WDEPTH=18, DEPTHT=17,
                CULLF=0, CULLBF=1, DEPTHF=2, BLENDE=5, BLENDF1=9, BLENDF2=13
            };};
    };

    inline uint DrawState::toGLBlendFunc(BlendFunc f)
    {
        switch(f)
        {
            case BlendFunc::CONSTANT_ALPHA:return GL_CONSTANT_ALPHA;
            case BlendFunc::CONSTANT_COLOR:return GL_CONSTANT_COLOR;
            case BlendFunc::DST_ALPHA:return GL_DST_ALPHA;
            case BlendFunc::ONE:return GL_ONE;
            case BlendFunc::ONE_MINUS_CONSTANT_ALPHA:return GL_ONE_MINUS_CONSTANT_ALPHA;
            case BlendFunc::ONE_MINUS_CONSTANT_COLOR:return GL_ONE_MINUS_CONSTANT_COLOR;
            case BlendFunc::ONE_MINUS_DST_ALPHA:return GL_ONE_MINUS_DST_ALPHA;
            case BlendFunc::ONE_MINUS_DST_COLOR:return GL_ONE_MINUS_DST_COLOR;
            case BlendFunc::ONE_MINUS_SRC_ALPHA:return GL_ONE_MINUS_SRC_ALPHA;
            case BlendFunc::ONE_MINUS_SRC_COLOR:return GL_ONE_MINUS_SRC_COLOR;
            case BlendFunc::SRC_ALPHA:return GL_SRC_ALPHA;
            case BlendFunc::SRC_COLOR:return GL_SRC_COLOR;
            case BlendFunc::DST_COLOR:return GL_DST_COLOR;
            case BlendFunc::ZERO:return GL_ZERO;
        }
        return GL_ONE_MINUS_SRC_ALPHA;
    }

    inline uint DrawState::toGLComparFunc(CompareFunc f)
    {
        switch(f)
        {
            case CompareFunc::ALWAYS:return GL_ALWAYS;
            case CompareFunc::NEVER:return GL_NEVER;
            case CompareFunc::NOTEQUAL:return GL_NOTEQUAL;
            case CompareFunc::LESS:return GL_LESS;
            case CompareFunc::LEQUAL:return GL_LEQUAL;
            case CompareFunc::GREATER:return GL_GREATER;
            case CompareFunc::GEQUAL:return GL_GEQUAL;
            case CompareFunc::EQUAL:return GL_EQUAL;
        }
        return GL_LESS;
    }

    inline uint DrawState::toGLBlendEquation(BlendEquation e)
    {
        switch(e)
        {
            case BlendEquation::ADD:return GL_FUNC_ADD;
            case BlendEquation::SUB:return GL_FUNC_SUBTRACT;
            case BlendEquation::REVERSE_SUB:return GL_FUNC_REVERSE_SUBTRACT;
            case BlendEquation::MIN:return GL_MIN;
            case BlendEquation::MAX:return GL_MAX;
        }
        return GL_FUNC_ADD;
    }
}
}
#include "MemoryLoggerOff.h"

#endif // DRAWSTAT_H_INCLUDED
