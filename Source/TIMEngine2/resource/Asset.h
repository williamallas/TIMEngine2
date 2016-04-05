#ifndef ASSET_H_INCLUDED
#define ASSET_H_INCLUDED

#include "core/core.h"
#include <boost/smart_ptr.hpp>

namespace tim
{
    using namespace core;
namespace resource
{
    template <class AssetType>
    class Asset
    {
    public:
        using Type = AssetType;

        Asset() = default;
        Asset(AssetType* ptr) : _ptr(ptr) {}
        Asset(const Asset&) = default;
        Asset(Asset&& asset) : _ptr(std::move(asset._ptr)) {}

        ~Asset() = default;

        Asset& operator=(const Asset&) = default;
        Asset& operator=(Asset&& asset) { _ptr=std::move(asset._ptr); return *this; }

        bool isNull() const { return _ptr.get() == nullptr; }

    protected:
        boost::shared_ptr<AssetType> _ptr;
    };
}
}

#endif // ASSET_H_INCLUDED
