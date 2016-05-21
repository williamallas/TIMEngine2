#ifndef XMLMESHASSETLOADER_H
#define XMLMESHASSETLOADER_H

#include <tinyxml.h>
#include "core/core.h"
#include "interface/Mesh.h"

#include "MemoryLoggerOn.h"
#undef interface
namespace tim
{
namespace interface
{

    class XmlMeshAssetLoader
    {
    public:
        XmlMeshAssetLoader();

        bool load(std::string);
        interface::Mesh getMesh(std::string, const renderer::Texture::GenTexParam&) const;

    protected:
        struct MeshElementModel
        {
            int type = 0;

            // type = 0
            vec4 material = {0.7f, 0, 0.1f, 0};
            vec3 color = {0.7f,0.7f,0.7f};
            std::string textures[3];
            std::string geometry;
        };

        boost::container::map<std::string, vector<MeshElementModel>> _models;

        void parseMeshAssetElement(TiXmlElement*);

        static vec3 toColor(std::string);
        static std::string str(const char*);
    };

}
}
#include "MemoryLoggerOff.h"

#endif // XMLMESHASSETLOADER_H
