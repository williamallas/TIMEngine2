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
        struct Collider
        {
            enum { NONE, AUTO_BOX, AUTO_SPHERE, CONVEX_HULL, USER_DEFINED };
            int type = NONE;

            float mass=1,
                  friction=1,
                  rollingFriction=1,
                  restitution=0.8;
        };

        struct ObjectLoaded
        {
            std::string name;
            std::string model;

            enum { MESH_INSTANCE, };
            int type = MESH_INSTANCE;

            MeshInstance* meshInstance;
            bool isStatic, isPhysic, isVisible;
            Collider collider;

            vector<XmlMeshAssetLoader::MeshElementModel> asset;
            mat3 rotation = mat3::IDENTITY();
            vec3 translation;
            vec3 scale = {1,1,1};
        };

        static bool loadScene(std::string, interface::Scene&, vector<ObjectLoaded>&);

        static void parseTransformation(TiXmlElement* elem, vec3& tr, vec3& sc, mat3& rot, Collider* collider);
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
