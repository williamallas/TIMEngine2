#ifndef SHADER_H
#define SHADER_H

#include "GLState.h"
#include "core/core.h"
#include "core/Matrix.h"
#include "core/Option.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    class Shader
    {
    public:
        static Option<Shader*> combine(const Option<uint>&, const Option<uint>&, const Option<uint>& gs=Option<uint>());
        static Option<Shader*> linkComputeShader(const Option<uint>&);
        static const std::string& lastLinkError();

        virtual ~Shader();
        uint id() const;

        enum EngineUniform
        {
            VIEW, PROJ, PROJVIEW, INV_VIEW, INV_PROJ, INV_PROJVIEW,
            WORLD_ORIGIN, INV_PROJVIEW_WORLD_ORIGIN,
            TIME, FRAME_TIME, NB_DRAW,
            CAMERA_WORLD, CAMERA_UP, CAMERA_DIR,
            LASTEngineUniform,
        };
        int engineUniformId(EngineUniform) const;
        int engineTextureScaleId(size_t) const;

        void bind();

        int uniformLocation(const std::string&) const;
        void setUniform(const mat4&, int) const;
        void setUniform(const mat3&, int) const;
        void setUniform(const vec4&, int) const;
        void setUniform(const vec3&, int) const;
        void setUniform(const vec2&, int) const;
        void setUniform(const ivec2&, int) const;
        void setUniform(const ivec3&, int) const;
        void setUniform(int, int) const;
        void setUniform(float, int) const;
        void setUniform(const mat4[], size_t, int) const;
        void setUniform(const vec4[], size_t, int) const;
        void setUniform(const ivec4[], size_t, int) const;

    private:
        Shader();
        uint _id;

        int _engineUniform[EngineUniform::LASTEngineUniform];
        int _uniformTextureId[MAX_TEXTURE_UNIT]={-1};
        int _uniformImageId[MAX_IMAGE_UNIT]={-1};

        #include "MemoryLoggerOff.h"
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        #include "MemoryLoggerOn.h"

        static std::string _lastLinkError;

        void loadEngineUniform();
    };

    /* inline implementation */
    inline uint Shader::id() const { return _id; }
    inline void Shader::bind() { openGL.bindShader(_id); }
    inline int Shader::engineUniformId(EngineUniform id) const { return _engineUniform[id]; }

    inline void Shader::setUniform(const mat4& x, int id) const { if(id>=0) glUniformMatrix4fv(id, 1, GL_TRUE, x.data()); }
    inline void Shader::setUniform(const mat3& x, int id) const { if(id>=0) glUniformMatrix3fv(id, 1, GL_TRUE, x.data()); }
    inline void Shader::setUniform(const vec4& x, int id) const { if(id>=0) glUniform4f(id, x.x(), x.y(), x.z(), x.w()); }
    inline void Shader::setUniform(const vec3& x, int id) const { if(id>=0) glUniform3f(id, x.x(), x.y(), x.z()); }
    inline void Shader::setUniform(const vec2& x, int id) const { if(id>=0) glUniform2f(id, x.x(), x.y()); }
    inline void Shader::setUniform(const ivec2& x, int id) const { if(id>=0) glUniform2i(id, x.x(), x.y()); }
    inline void Shader::setUniform(const ivec3& x, int id) const { if(id>=0) glUniform3i(id, x.x(), x.y(), x.z()); }
    inline void Shader::setUniform(int x, int id) const { if(id>=0) glUniform1i(id, x); }
    inline void Shader::setUniform(float x, int id) const { if(id>=0) glUniform1f(id, x); }
    inline void Shader::setUniform(const mat4* x, size_t s, int id) const { if(id>=0) glUniformMatrix4fv(id, s, GL_TRUE, x[0].data()); }
    inline void Shader::setUniform(const vec4* x, size_t s, int id) const { if(id>=0) glUniform4fv(id, s, x[0].data()); }
    inline void Shader::setUniform(const ivec4* x, size_t s, int id) const { if(id>=0) glUniform4iv(id, s, x[0].data()); }
    inline int Shader::uniformLocation(const std::string& s) const { return glGetUniformLocation(_id, s.c_str()); }
}
}

#include "MemoryLoggerOff.h"

#endif // SHADER_H
