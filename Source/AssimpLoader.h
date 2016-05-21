#ifndef ASSIMPMESHLOADER_H
#define ASSIMPMESHLOADER_H

#include "core/core.h"
#include "core/Matrix.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "interface/XmlMeshAssetLoader.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    class AssimpLoader
    {
    public:
        struct Node
        {
            std::string name, idName;
            mat4 matrix;
        };

        AssimpLoader();
        virtual ~AssimpLoader();

        bool load(const std::string&);
        const vector<Node>& nodes() const;
        vector<Node> findNodeWithIdName(std::string) const;
        Node findNode(std::string) const;

    private:
        Assimp::Importer importer;
        vector<Node> _nodes;

        void processSceneNode(aiNode*, mat4 m=mat4::IDENTITY());

        static mat4 convert(const aiMatrix4x4&);
        static vec3 convert(const aiVector3D&);
        static vec4 convert(const aiQuaternion&);

    };

    inline const vector<AssimpLoader::Node>& AssimpLoader::nodes() const
    {
        return _nodes;
    }
}

#include "MemoryLoggerOff.h"

#endif // ASSIMPMESHLOADER_H
