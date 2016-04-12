#include "ShaderPool.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{

using namespace renderer;

ShaderPool::ShaderPool()
{

}

ShaderPool::~ShaderPool()
{
    for(auto p : _shaders)
    {
        delete p.second;
    }
}

Option<renderer::Shader*> ShaderPool::add(std::string name, std::string vs, std::string ps, std::string gs)
{
    Option<uint> vShader, pShader, gShader;

    ShaderCompiler vsCompiler(ShaderCompiler::VERTEX_SHADER);
    vsCompiler.setSource(StringUtils::readFile(vs));
    vShader = vsCompiler.compile({});
    if(!vShader.hasValue())
        LOG("Error compiling ",vs," :\n",vsCompiler.error());

    ShaderCompiler psCompiler(ShaderCompiler::PIXEL_SHADER);
    if(!ps.empty())
    {
        psCompiler.setSource(StringUtils::readFile(ps));
        pShader = psCompiler.compile({});
        if(!pShader.hasValue())
            LOG("Error compiling ",ps," :\n",psCompiler.error());
    }

    ShaderCompiler gsCompiler(ShaderCompiler::GEOMETRY_SHADER);
    if(!gs.empty())
    {
        gsCompiler.setSource(StringUtils::readFile(gs));
        gShader = gsCompiler.compile({});
        if(!gShader.hasValue())
            LOG("Error compiling ",gs," :\n",gsCompiler.error());
    }

    auto optShader = Shader::combine(vShader, pShader, gShader);

    if(!optShader.hasValue())
        LOG("Error when linking: ",vs, " - ", ps, " - ", gs, " :\n", Shader::lastLinkError());
    else
        _shaders[name] = optShader.value();

    return optShader;
}

Option<renderer::Shader*> ShaderPool::addCompute(std::string name, std::string cs)
{
    ShaderCompiler csCompiler(ShaderCompiler::COMPUTE_SHADER);
    csCompiler.setSource(StringUtils::readFile(cs));
    Option<uint> csShader = csCompiler.compile({});
    if(!csShader.hasValue())
        LOG("Error compiling ", cs, " :\n", csCompiler.error());

    auto optShader = Shader::linkComputeShader(csShader);

    if(!optShader.hasValue())
        LOG("Error when linking: ", cs, " :\n", Shader::lastLinkError());
    else
        _shaders[name] = optShader.value();

    return optShader;
}

renderer::Shader* ShaderPool::get(std::string n) const
{
    return _shaders.at(n);
}

}
}


