#ifndef XMLSCENELOADER_H
#define XMLSCENELOADER_H

#include "XmlMeshAssetLoader.h"
#include "pipeline/pipeline.h"
#include "MeshInstance.h"

#undef tim
#include "MemoryLoggerOn.h"
#undef interface
namespace tim
{
namespace interface
{
    class XmlSceneLoader
    {
    public:
        struct ObjectLoaded
        {
            std::string name;
            std::string model;

            enum { MESH_INSTANCE, };
            int type = MESH_INSTANCE;

            MeshInstance* meshInstance;
            bool isStatic, isPhysic;

            vector<XmlMeshAssetLoader::MeshElementModel> asset;
        };

        static bool loadScene(std::string, interface::Scene&, vector<ObjectLoaded>&);

        static void parseTransformation(TiXmlElement* elem, vec3& tr, vec3& sc, mat3& rot);
        static vector<std::string> parseSkyboxXmlElement(TiXmlElement* elem);

    private:
        #include "MemoryLoggerOff.h"
        XmlSceneLoader() = delete;
        #include "MemoryLoggerOn.h"

    };
}
}
#include "MemoryLoggerOff.h"

#endif // XMLSCENELOADER_H
