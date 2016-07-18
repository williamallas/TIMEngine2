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
        struct MeshElementModel
        {
            int type = 0;

            // type = 0
            vec4 material = {0.7f, 0, 0.1f, 0};
            vec3 color = {0.7f,0.7f,0.7f};
            std::string textures[3];
            std::string geometry;
        };

        XmlMeshAssetLoader();

        void addModel(std::string, const vector<MeshElementModel>&);
        bool load(std::string);
        interface::Mesh getMesh(std::string, const renderer::Texture::GenTexParam&) const;

        const boost::container::map<std::string, vector<MeshElementModel>>& allAssets() const { return _models; }

        static vector<MeshElementModel> parseMeshAssetElement(TiXmlElement*, std::string& name);

    protected:

        boost::container::map<std::string, vector<MeshElementModel>> _models;

        static vec3 toColor(std::string);
    };

}
}
#include "MemoryLoggerOff.h"

#endif // XMLMESHASSETLOADER_H
