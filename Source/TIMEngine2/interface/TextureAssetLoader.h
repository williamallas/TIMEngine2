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

            if(!texData)
            {
                LOG_EXT("Unable to load texture ", file);
                return Option<interface::Texture>();
            }

            renderer::Texture* tex = renderer::Texture::genTexture2D(param, texData, imgLoaded.nbComponent);
            tex->makeBindless();
            delete[] texData;
            return Option<interface::Texture>(tex);
        }

        template<bool async>
        Option<interface::Texture> operator()(const vector<std::string>& file, renderer::Texture::GenTexParam param)
        {
            static_assert(!async, "Async texture loading not supported");

            if(resource::textureLoader == nullptr || file.empty())
            {
                LOG_EXT("resource::textureLoader is null");
                return Option<interface::Texture>();
            }

            uivec2 res;
            uint nbComponent;
            vector<ubyte*> datas;
            for(uint i=0 ; i<file.size() ; ++i)
            {
                TextureLoader::ImageFormat imgLoaded;
                ubyte* texData = resource::textureLoader->loadImage(file[i], imgLoaded);

                if(i == 0)
                {
                    nbComponent = imgLoaded.nbComponent;
                    res = imgLoaded.size;
                }
                else if(res != imgLoaded.size || nbComponent != imgLoaded.nbComponent)
                {
                    delete[] texData;
                    break;
                }

                if(texData)
                    datas.push_back(texData);
            }

            ubyte* concatData = new ubyte[res.x()*res.y()*nbComponent*datas.size()];

            for(size_t i=0 ; i<datas.size() ; ++i)
            {
                std::memcpy(concatData+res.x()*res.y()*nbComponent*i, datas[i], res.x()*res.y()*nbComponent);
                delete[] datas[i];
            }

            param.size = uivec3(res,datas.size());
            renderer::Texture* tex = renderer::Texture::genTextureArray2D(param, concatData, nbComponent);
            tex->makeBindless();
            delete[] concatData;
            return Option<interface::Texture>(tex);
        }

    private:
    };
}
}

#endif // TEXTUREASSETLOADER_H_INCLUDED
