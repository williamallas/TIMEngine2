#include "ShaderCompiler.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

GLenum ShaderCompiler::GLShaderType(ShaderType type)
{
    switch(type)
    {
        case VERTEX_SHADER: return GL_VERTEX_SHADER;
        case PIXEL_SHADER : return GL_FRAGMENT_SHADER;
        case GEOMETRY_SHADER : return GL_GEOMETRY_SHADER;
        case COMPUTE_SHADER : return GL_COMPUTE_SHADER;
    }
    return GL_VERTEX_SHADER;
}

ShaderCompiler::ShaderCompiler(ShaderType shaderType)
{
    _shaderType = shaderType;
}

ShaderCompiler::~ShaderCompiler()
{
    for(auto &s : _shader)
    {
        glDeleteShader(s.second);
    }
}

void ShaderCompiler::setSource(const std::string& str)
{
    _source = str;
}

Option<core::uint> ShaderCompiler::compile(std::initializer_list<std::string> flags)
{
    return compile(boost::container::set<std::string>(flags.begin(), flags.end()));
}

Option<core::uint> ShaderCompiler::compile(const boost::container::set<std::string>& flags)
{
    auto it = _shader.find(flags);
    if(it != _shader.end())
        return Option<core::uint>(it->second);

    std::string finalSource=_source;
    size_t pos = finalSource.find("#version");
    pos = finalSource.find("\n", pos);

    std::string defineStat;
    for(auto it=flags.begin() ; it != flags.end() ; ++it)
        defineStat += "#define "+*it+"\n";

    defineStat += getBuiltInDefine();

    if(pos != std::string::npos)
        finalSource.insert(pos+1, defineStat);

    core::uint id = glCreateShader(GLShaderType(_shaderType));
    const GLchar* gchar = (const GLchar*)finalSource.c_str();
    glShaderSource(id, 1, &gchar, NULL);
    glCompileShader(id);

    int compileStatus = GL_TRUE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus);

    if(compileStatus != GL_TRUE)
    {
        _lastError="Compiling with:";
        for(auto it=flags.begin() ; it != flags.end() ; ++it)
            _lastError += *it +" ";
        _lastError += "\n";
        logError(id);
        glDeleteShader(id);
        return Option<core::uint>();
    }

    _shader[flags] = id;

    return Option<core::uint>(id);
}

void ShaderCompiler::logError(core::uint shaderId)
{
    int logSize;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logSize);

    char* log = new char[logSize];
    glGetShaderInfoLog(shaderId, logSize, &logSize, log);

    _lastError += std::string(log);
    delete[] log;
}

std::string ShaderCompiler::getBuiltInDefine()
{
    static std::string define =
            "#define MAX_UBO_VEC4 " + StringUtils(openGL.hardward(GLState::Hardward::MAX_UNIFORM_BLOCK_SIZE) / 16).str() +
            "\n";
    return define;

}

}
}
