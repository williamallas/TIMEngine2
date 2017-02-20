#ifndef XMLMESHASSETLOADER_H
#define XMLMESHASSETLOADER_H

#include <tinyxml.h>
#include "core/core.h"
#include "interface/Mesh.h"
#include "renderer/DrawState.h"

#include "MemoryLoggerOn.h"
#undef interface
#undef DrawState
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
            float textureScale = 1;
            std::string textures[3];
            std::string geometry;

            bool useAdvanced = false;
            std::string advancedShader;
            renderer::DrawState advanced;
            bool castShadow = true;
            bool cmAffected = true;
        };

        XmlMeshAssetLoader();

        void addModel(std::string, const vector<MeshElementModel>&);
        bool load(std::string);
        interface::Mesh getMesh(std::string, const renderer::Texture::GenTexParam&) const;

        static interface::Mesh constructMesh(const vector<MeshElementModel>&, const renderer::Texture::GenTexParam&, bool keepData=true);

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
