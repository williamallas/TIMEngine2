#include "ObjectModelManager.h"

#include "MemoryLoggerOn.h"
namespace tim
{

bool ObjectModelManager::load(const std::string& file)
{
    TiXmlDocument doc(PATH_DATA+file);

    if(!doc.LoadFile())
        return false;

    TiXmlElement* elem=doc.FirstChildElement();

    while(elem)
    {
        const char* name = elem->Attribute("name");
        if(elem->ValueStr() == "Object" && name)
            parseEditorModel(elem->FirstChildElement(), std::string(name));

        elem=elem->NextSiblingElement();
    }
    return true;
}

bool ObjectModelManager::loadScene(const std::string& file, const mat4& trans, LoadSceneType& mapName) const
{
    TiXmlDocument doc(PATH_DATA+file);

    if(!doc.LoadFile())
        return false;

    TiXmlElement* elem=doc.FirstChildElement();

    while(elem)
    {
        if(elem->ValueStr() == "Scene")
            parseAddObjectMatrix(elem->FirstChildElement(), trans, mapName);

        elem=elem->NextSiblingElement();
    }
    return true;
}

interface::RenderableObject* ObjectModelManager::add(const std::string& str, const mat4& matrix, interface::LightObject** light) const
{
    auto it = _models.find(str);
    if(it == _models.end())
        return nullptr;

    const Model& model = it->second;
    interface::RenderableObject* obj = _engine->scene.addRenderableObject(matrix);

    if(light)
    {
        if(model.hasLight)
        {
            (*light) = _engine->scene.addLightObject(matrix*mat4::Translation(model.lightPosition));
            (*light)->light().setLightData(model.light);
        }
        else light = nullptr;
    }

    for(int i=0 ; i<RENDERABLE_OBJECT_NB_LOD ; ++i)
    {
        for(size_t j=0 ; j<model.model[i].size() ; ++j)
           obj->addElement(_engine->resource_manager.getMaterialPass(model.model[i][j]), i);

        if(model.model[i].size() > 0)
            obj->setLodDistance(model.lodDist[i], i);
    }

    return obj;
}

vector<renderer::MeshBuffers*> ObjectModelManager::getObjectGeometry(const std::string& str, int lod) const
{
    auto it = _models.find(str);
    if(it == _models.end())
        return {};

    const Model& model = it->second;
    vector<renderer::MeshBuffers*> result;
    if(lod == -1)
    {
        for(int i=RENDERABLE_OBJECT_NB_LOD-1 ; i>=0 ; --i)
        {
            if(model.model[i].size() > 0)
            {
                for(size_t j=0 ; j<model.model[i].size() ; ++j)
                {
                    renderer::MaterialPass* pass = _engine->resource_manager.asyncTexture_getMaterialPass(model.model[i][j]);
                    if(pass && pass->getPass(0) && pass->getPass(0)->mesh())
                        result.push_back(pass->getPass(0)->mesh());
                }
                return result;
            }
        }
    }
    return result;
}

void ObjectModelManager::parseEditorModel(TiXmlElement* elem, std::string name)
{
    Model v;
    while(elem)
    {
        if(elem->ValueStr() == "mesh")
        {
            int lod=0;
            elem->QueryIntAttribute("lod", &lod);
            elem->QueryFloatAttribute("distance", &v.lodDist[lod]);
            lod = std::min(std::max(lod, 0), RENDERABLE_OBJECT_NB_LOD-1);
            v.model[lod].push_back(elem->GetText());
        }
        else if(elem->ValueStr() == "light")
        {
            v.hasLight=true;

            v.light.diffuse = toVec<4>(StringUtils::str(elem->Attribute("diffuse")));
            v.light.specular = toVec<4>(StringUtils::str(elem->Attribute("specular")));
            v.light.attenuation = toVec<4>(StringUtils::str(elem->Attribute("attenuation")));
            v.lightPosition = toVec<3>(StringUtils::str(elem->Attribute("position")));
        }

        elem = elem->NextSiblingElement();
    }

    _models[name]= v;
}

void ObjectModelManager::parseAddObjectMatrix(TiXmlElement* elem, const mat4& trans, LoadSceneType& nameObj) const
{
    while(elem)
    {
        if(elem->ValueStr() == "Object")
        {
            TiXmlElement* child = elem->FirstChildElement();
            std::string name_data, name;
            mat4 m = mat4::IDENTITY();
            name = StringUtils::str(elem->Attribute("name"));
            //bool useModel=true;
            while(child)
            {
                if(child->ValueStr() == "model")
                {
                    name_data = StringUtils::str(child->GetText());
                    //useModel = true;
                }
                else if(child->ValueStr() == "matrix")
                {
                    std::stringstream ss(StringUtils::str(child->GetText()));
                    for(int i=0 ; i<16 ; ++i)
                        ss >> m.get(i);

                    m = trans * m;
                    out(m)<<std::endl;
                }
                child = child->NextSiblingElement();
            }

            interface::LightObject* light=nullptr;
            interface::RenderableObject* obj = add(name_data, m, &light);
            nameObj[name] = std::make_pair(obj, getObjectGeometry(name_data));
            if(light)
                nameObj[name+"_light"] = std::make_pair(light, vector<renderer::MeshBuffers*>());
        }
        elem = elem->NextSiblingElement();
    }
}

}
