#include "ResourceManager.h"

namespace tim
{
    using namespace core;
namespace interface
{

resource::AssetManager<interface::Geometry>& geometryManager = resource::AssetManager<Geometry>::instance();
resource::AssetManager<interface::Texture>& textureManager = resource::AssetManager<Texture>::instance();
interface::ShaderPool& shaderManager = ShaderPool::instance();

}
}
