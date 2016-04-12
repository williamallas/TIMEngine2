#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "resource/AssetManager.h"
#include "interface/Geometry.h"
#include "interface/Texture.h"
#include "interface/ShaderPool.h"

namespace tim
{
    using namespace core;
namespace interface
{

extern resource::AssetManager<interface::Geometry>& geometryManager;
extern resource::AssetManager<interface::Texture>& textureManager;
extern interface::ShaderPool& shaderManager;

}
}

#endif // RESOURCEMANAGER_H
