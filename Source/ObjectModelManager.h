#ifndef OBJECTMODEL_H_INCLUDED
#define OBJECTMODEL_H_INCLUDED

#include "interface/EnginePackage.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    class ObjectModelManager
    {
    public:
        struct Model
        {
            vector<std::string> model[RENDERABLE_OBJECT_NB_LOD];
            float lodDist[RENDERABLE_OBJECT_NB_LOD] = {0,50,100,200};
            bool hasLight = false;
            renderer::Light::LightData light;
            vec3 lightPosition;
        };

        using LoadSceneType = boost::container::map<std::string, std::pair<interface::SceneObject*, vector<renderer::MeshBuffers*>>>;

        ObjectModelManager(interface::EnginePackage* engine) : _engine(engine) {}
        bool load(const std::string& file);

        bool loadScene(const std::string&, const mat4&, LoadSceneType&) const;

        interface::RenderableObject* add(const std::string&, const mat4&, interface::LightObject** light=nullptr) const;
        vector<renderer::MeshBuffers*> getObjectGeometry(const std::string&, int lod=-1) const;

    private:
        interface::EnginePackage* _engine;
        boost::container::map<std::string, Model> _models;

        void parseEditorModel(TiXmlElement*, std::string);
        void parseAddObjectMatrix(TiXmlElement*, const mat4&, LoadSceneType&) const;

    };
}
#include "MemoryLoggerOff.h"

#endif // OBJECTMODEL_H_INCLUDED
