#ifndef DRAWSTATE_H_INCLUDED
#define DRAWSTATE_H_INCLUDED

#include "GLState.h"
#include "Shader.h"
#include <cstdint>

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
            ALWAYS
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

        enum Primitive : uint
        {
            TRIANGLES=0,
            TRIANGLE_STRIP,
            LINES,
            LINE_STRIP,
            POINTS
        };

        static uint toGLBlendFunc(BlendFunc);
        static uint toGLComparFunc(CompareFunc);
        static uint toGLBlendEquation(BlendEquation);
        static uint toGLPrimitive(Primitive);

        static BlendFunc fromBlendFuncStr(std::string);
        static CompareFunc fromComparFuncStr(std::string);
        static BlendEquation fromBlendEquationStr(std::string);
        static Primitive fromPrimitiveStr(std::string);

        static std::string toBlendFuncStr(BlendFunc);
        static std::string toComparFuncStr(CompareFunc);
        static std::string toBlendEquationStr(BlendEquation);
        static std::string toPrimitiveStr(Primitive);

        DrawState()
        {
           setPriority(128);
           setBlend(false);
           setCullBackFace(true);
           setCullFace(true);
           setDepthTest(true);
           setWriteDepth(true);
           setDepthFunc(LESS);
           setBlendEquation(ADD);
           setBlendFunc({SRC_ALPHA, ONE_MINUS_SRC_ALPHA});
           setPrimitive(TRIANGLES);
        }

        ~DrawState() = default;

        void setShader(Shader* shader) { _shader = shader; }

        void setPriority(ubyte p)     { _data.priority = p; }
        void setBlend(bool b)        { _data.blend = b; }
        void setWriteDepth(bool b)   { _data.writeDepth = b; }
        void setDepthTest(bool b)    { _data.depthTest = b; }

        void setCullFace(bool b)     { _data.cullFace = b; }
        void setCullBackFace(bool b) { _data.cullBackFace = b; }

        void setDepthFunc(CompareFunc c) { _data.depthFunc = c; }
        void setBlendEquation(BlendEquation be) { _data.blendEqu = be; }

        void setBlendFunc(const Vector2<BlendFunc>& funcs)
        {
            _data.blendFunc1 = funcs[0];
            _data.blendFunc2 = funcs[1];
        }

        void setPrimitive(Primitive p) { _data.primitive = p; }

        Shader* shader() const { return _shader; }

        ubyte priority()    const { return _data.priority; }
        bool blend()        const { return  _data.blend; }
        bool writeDepth()   const { return  _data.writeDepth; }
        bool depthTest()    const { return  _data.depthTest; }
        bool cullFace()     const { return  _data.cullFace; }
        bool cullBackFace() const { return  _data.cullBackFace; }

        CompareFunc depthFunc() const { return static_cast<CompareFunc>(_data.depthFunc); }
        BlendEquation blendEquation() const { return static_cast<BlendEquation>(_data.blendEqu); }

        Vector2<BlendFunc> blendFunc() const
        {
            return Vector2<BlendFunc>(static_cast<BlendFunc>(_data.blendFunc1),
                                      static_cast<BlendFunc>(_data.blendFunc2));
        }

        Primitive primitive() const { return static_cast<Primitive>(_data.primitive); }

        bool operator<(const DrawState& state) const
        {
            if(_packed != state._packed) return _packed < state._packed;
            else return _shader < state._shader;
        }

        bool operator==(const DrawState& state) const
        {
            return _packed==state._packed && _shader == state._shader;
        }

        bool operator!=(const DrawState& state) const { return !(*this==state); }

        void bind() const
        {
            if(_shader != nullptr)
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
                   << "primitive: " << primitive() << "\n"
                   << "priority : " << priority() << "\n"
                   << "byte     : " << _packed << "\n";
        }

    private:

        struct BitField
        {
            unsigned priority      : 8;
            unsigned blend         : 1;
            unsigned primitive     : 3;
            unsigned writeDepth    : 1;
            unsigned depthTest     : 1;
            unsigned cullFace      : 1;
            unsigned cullBackFace  : 1;
            unsigned depthFunc     : 3;
            unsigned garbage       : 1;
            unsigned blendEqu      : 4;
            unsigned blendFunc1    : 4;
            unsigned blendFunc2    : 4;
        }__attribute__((packed));

        union
        {
            BitField _data;
            uint32_t _packed;
        };

        Shader* _shader = nullptr;

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

    inline uint DrawState::toGLPrimitive(Primitive e)
    {
        switch(e)
        {
            case Primitive::TRIANGLES:return GL_TRIANGLES;
            case Primitive::TRIANGLE_STRIP:return GL_TRIANGLE_STRIP;
            case Primitive::LINES:return GL_LINES;
            case Primitive::LINE_STRIP:return GL_LINE_STRIP;
            case Primitive::POINTS:return GL_POINTS;
        }
        return GL_TRIANGLES;
    }

    inline DrawState::BlendFunc DrawState::fromBlendFuncStr(std::string str)
    {
        if(str == "CONSTANT_ALPHA") return BlendFunc::CONSTANT_ALPHA;
        else if(str == "CONSTANT_COLOR") return BlendFunc::CONSTANT_COLOR;
        else if(str == "DST_ALPHA") return BlendFunc::DST_ALPHA;
        else if(str == "ONE") return BlendFunc::ONE;
        else if(str == "ONE_MINUS_CONSTANT_ALPHA") return BlendFunc::ONE_MINUS_CONSTANT_ALPHA;
        else if(str == "ONE_MINUS_CONSTANT_COLOR") return BlendFunc::ONE_MINUS_CONSTANT_COLOR;
        else if(str == "ONE_MINUS_DST_ALPHA") return BlendFunc::ONE_MINUS_DST_ALPHA;
        else if(str == "ONE_MINUS_DST_COLOR") return BlendFunc::ONE_MINUS_DST_COLOR;
        else if(str == "ONE_MINUS_SRC_ALPHA") return BlendFunc::ONE_MINUS_SRC_ALPHA;
        else if(str == "ONE_MINUS_SRC_COLOR") return BlendFunc::ONE_MINUS_SRC_COLOR;
        else if(str == "SRC_ALPHA") return BlendFunc::SRC_ALPHA;
        else if(str == "SRC_COLOR") return BlendFunc::SRC_COLOR;
        else if(str == "DST_COLOR") return BlendFunc::DST_COLOR;
        else if(str == "ZERO") return BlendFunc::ZERO;
        else return BlendFunc::SRC_ALPHA;
    }

    inline DrawState::CompareFunc DrawState::fromComparFuncStr(std::string str)
    {
        if(str == "ALWAYS") return CompareFunc::ALWAYS;
        else if(str == "NEVER") return CompareFunc::NEVER;
        else if(str == "NOTEQUAL") return CompareFunc::NOTEQUAL;
        else if(str == "LESS") return CompareFunc::LESS;
        else if(str == "LEQUAL") return CompareFunc::LEQUAL;
        else if(str == "GREATER") return CompareFunc::GREATER;
        else if(str == "GEQUAL") return CompareFunc::GEQUAL;
        else if(str == "EQUAL") return CompareFunc::EQUAL;
        else return CompareFunc::LESS;
    }

    inline DrawState::BlendEquation DrawState::fromBlendEquationStr(std::string str)
    {
        if(str == "ADD") return BlendEquation::ADD;
        else if(str == "SUB") return BlendEquation::SUB;
        else if(str == "REVERSE_SUB") return BlendEquation::REVERSE_SUB;
        else if(str == "MIN") return BlendEquation::MIN;
        else if(str == "MAX") return BlendEquation::MAX;
        else return BlendEquation::ADD;
    }

    inline DrawState::Primitive DrawState::fromPrimitiveStr(std::string str)
    {
        if(str == "TRIANGLES") return Primitive::TRIANGLES;
        else if(str == "TRIANGLE_STRIP") return Primitive::TRIANGLE_STRIP;
        else if(str == "LINES") return Primitive::LINES;
        else if(str == "LINE_STRIP") return Primitive::LINE_STRIP;
        else if(str == "POINTS") return Primitive::POINTS;
        else return Primitive::TRIANGLES;
    }

    inline std::string DrawState::toBlendFuncStr(BlendFunc f)
    {
        switch(f)
        {
            case BlendFunc::CONSTANT_ALPHA:return "CONSTANT_ALPHA";
            case BlendFunc::CONSTANT_COLOR:return "CONSTANT_COLOR";
            case BlendFunc::DST_ALPHA:return "DST_ALPHA";
            case BlendFunc::ONE:return "ONE";
            case BlendFunc::ONE_MINUS_CONSTANT_ALPHA:return "ONE_MINUS_CONSTANT_ALPHA";
            case BlendFunc::ONE_MINUS_CONSTANT_COLOR:return "ONE_MINUS_CONSTANT_COLOR";
            case BlendFunc::ONE_MINUS_DST_ALPHA:return "ONE_MINUS_DST_ALPHA";
            case BlendFunc::ONE_MINUS_DST_COLOR:return "ONE_MINUS_DST_COLOR";
            case BlendFunc::ONE_MINUS_SRC_ALPHA:return "ONE_MINUS_SRC_ALPHA";
            case BlendFunc::ONE_MINUS_SRC_COLOR:return "ONE_MINUS_SRC_COLOR";
            case BlendFunc::SRC_ALPHA:return "SRC_ALPHA";
            case BlendFunc::SRC_COLOR:return "SRC_COLOR";
            case BlendFunc::DST_COLOR:return "DST_COLOR";
            case BlendFunc::ZERO:return "ZERO";
        }
        return "ONE_MINUS_SRC_ALPHA";
    }

    inline std::string DrawState::toComparFuncStr(CompareFunc f)
    {
        switch(f)
        {
            case CompareFunc::ALWAYS:return "ALWAYS";
            case CompareFunc::NEVER:return "NEVER";
            case CompareFunc::NOTEQUAL:return "NOTEQUAL";
            case CompareFunc::LESS:return "LESS";
            case CompareFunc::LEQUAL:return "LEQUAL";
            case CompareFunc::GREATER:return "GREATER";
            case CompareFunc::GEQUAL:return "GEQUAL";
            case CompareFunc::EQUAL:return "EQUAL";
        }
        return "LESS";
    }

    inline std::string DrawState::toBlendEquationStr(BlendEquation e)
    {
        switch(e)
        {
            case BlendEquation::ADD:return "ADD";
            case BlendEquation::SUB:return "SUB";
            case BlendEquation::REVERSE_SUB:return "REVERSE_SUB";
            case BlendEquation::MIN:return "MIN";
            case BlendEquation::MAX:return "MAX";
        }
        return "ADD";
    }

    inline std::string DrawState::toPrimitiveStr(Primitive p)
    {
        switch(p)
        {
            case Primitive::TRIANGLES:return "TRIANGLES";
            case Primitive::TRIANGLE_STRIP:return "TRIANGLE_STRIP";
            case Primitive::LINES:return "LINES";
            case Primitive::LINE_STRIP:return "LINE_STRIP";
            case Primitive::POINTS:return "POINTS";
        }
        return "TRIANGLES";
    }
}
}
#include "MemoryLoggerOff.h"

#endif // DRAWSTAT_H_INCLUDED
