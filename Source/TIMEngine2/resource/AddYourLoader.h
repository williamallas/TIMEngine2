#ifndef ADDYOURLOADER_H_INCLUDED
#define ADDYOURLOADER_H_INCLUDED

#include "AssetLoader.h"
#include "interface/GeometryAssetLoader.h"
#include "interface/TextureAssetLoader.h"

#ifndef NO_OPEN_AL
#include "SoundAsset.h"
namespace tim {
    using namespace core;
namespace resource {

    template <>
    class AssetLoader<SoundAsset>
    {
    public:
        template<bool async>
        Option<SoundAsset> operator()(std::string file, bool stream, Sampler::Conversion conv = Sampler::LEFT)
        {
            static_assert(!async, "Async soundasset loading not supported");

            Sampler* sampler = new Sampler;
            bool ok = sampler->load(file, conv);
            if(ok)
            {
                Sound * sound = new Sound(sampler, stream);
                return Option<SoundAsset>(sound);
            }
            else
            {
                delete sampler;
                return Option<SoundAsset>();
            }
        }
    };

}
}
#endif

#endif // ADDYOURLOADER_H_INCLUDED
