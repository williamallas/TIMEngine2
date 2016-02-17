#ifndef METASHADER_H
#define METASHADER_H

#include "GLState.h"
#include "core/Option.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

    class ShaderCompiler
    {
    public:
        enum ShaderType
        {
            VERTEX_SHADER,
            PIXEL_SHADER,
            GEOMETRY_SHADER,
            COMPUTE_SHADER,
        };

        static GLenum GLShaderType(ShaderType);

        ShaderCompiler(ShaderType);
        virtual ~ShaderCompiler();

        ShaderType type() const;

        void setSource(const std::string&);

        Option<core::uint> compile(std::initializer_list<std::string>);
        Option<core::uint> compile(const boost::container::set<std::string>&);

        const std::string& error() const;

    private:
        ShaderType _shaderType;
        std::string _source;
        std::string _lastError;
        boost::container::map<boost::container::set<std::string>, core::uint> _shader;

        void logError(core::uint);
        static std::string getBuiltInDefine();

    };

    inline const std::string& ShaderCompiler::error() const { return _lastError; }

}
}
#include "MemoryLoggerOff.h"


#endif // METASHADER_H
