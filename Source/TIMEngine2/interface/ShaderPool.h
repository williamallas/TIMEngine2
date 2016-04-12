#ifndef SHADERPOOL_H
#define SHADERPOOL_H

#include "Singleton.h"
#include "renderer/ShaderCompiler.h"
#include "renderer/Shader.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{

    class ShaderPool : public Singleton<ShaderPool>
    {
    public:
        ShaderPool();
         ~ShaderPool();

        Option<renderer::Shader*> add(std::string name, std::string vs, std::string ps, std::string gs="");
        Option<renderer::Shader*> addCompute(std::string name, std::string cs);

        renderer::Shader* get(std::string) const;

    private:
        boost::container::map<std::string, renderer::Shader*> _shaders;
    };
}
}
#include "MemoryLoggerOff.h"

#endif // SHADERPOOL_H
