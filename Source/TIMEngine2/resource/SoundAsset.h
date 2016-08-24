#ifndef SOUNDASSET_H_INCLUDED
#define SOUNDASSET_H_INCLUDED

#ifndef NO_OPEN_AL

#include "core/core.h"
#include "openAL/Sound.hpp"
#include "Asset.h"

namespace tim
{
    using namespace core;
namespace resource
{

    class SoundAsset : public Asset<Sound>
    {
    public:
        using Asset<Sound>::Asset;

        SoundAsset() = default;
        SoundAsset(Sound* ptr) : Asset<Sound>(ptr) {}

        Sound* sound() const { return _ptr.get(); }
    };
}
}

#endif

#endif // SOUNDASSET_H_INCLUDED
