#ifndef TEXTUREASSETLOADER_H_INCLUDED
#define TEXTUREASSETLOADER_H_INCLUDED

#include "resource/TextureLoader.h"
#include "resource/AssetLoader.h"
#include "Texture.h"

namespace tim
{
    using namespace core;
namespace resource
{
    template <>
    class AssetLoader<interface::Texture>
    {
    public:
        template<bool async>
        Option<interface::Texture> operator()(std::string file, renderer::Texture::GenTexParam param)
        {
            static_assert(!async, "Async texture loading not supported");

            if(resource::textureLoader == nullptr)
            {
                LOG_EXT("resource::textureLoader is null");
                return Option<interface::Texture>();
            }

            TextureLoader::ImageFormat imgLoaded;
            ubyte* texData = resource::textureLoader->loadImage(file, imgLoaded);
            param.size = uivec3(imgLoaded.size,0);

            if(!texData) return Option<interface::Texture>();

            renderer::Texture* tex = renderer::Texture::genTexture2D(param, texData, imgLoaded.nbComponent);
            tex->makeBindless();
            return Option<interface::Texture>(tex);
        }

    private:
    };
}
}

#endif // TEXTUREASSETLOADER_H_INCLUDED
