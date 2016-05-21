#include "XmlMeshAssetLoader.h"

#include "interface/ShaderPool.h"
#include "resource/AssetManager.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace interface
{

XmlMeshAssetLoader::XmlMeshAssetLoader()
{

}

bool XmlMeshAssetLoader::load(std::string filename)
{
    TiXmlDocument doc(filename);

    if(!doc.LoadFile())
        return false;

    TiXmlElement* elem=doc.FirstChildElement();

    while(elem)
    {
        if(elem->ValueStr() == std::string("MeshAsset"))
            parseMeshAssetElement(elem);

        elem=elem->NextSiblingElement();
    }
    return true;
}

Mesh XmlMeshAssetLoader::getMesh(std::string name, const renderer::Texture::GenTexParam& texParam) const
{
    auto it = _models.find(name);
    if(it == _models.end())
        return interface::Mesh();

    vector<MeshElementModel> model = it->second;

    Mesh mesh;
    for(uint i=0 ; i<model.size() ; ++i)
    {
        Mesh::Element elem;
        if(model[i].type==0)
        {
            elem.setColor(vec4(model[i].color,1));
            elem.setRougness(model[i].material[0]);
            elem.setMetallic(model[i].material[1]);
            elem.setSpecular(model[i].material[2]);
            elem.setEmissive(model[i].material[3]);

            if(!model[i].geometry.empty())
                elem.setGeometry(resource::AssetManager<Geometry>::instance().load<false>(model[i].geometry, true).value());

            for(int j=0 ; j<3 ; ++j)
            {
                if(!model[i].textures[j].empty())
                    elem.setTexture(resource::AssetManager<Texture>::instance().load<false>(model[i].textures[j], texParam).value(), j);
            }
            elem.drawState().setShader(interface::ShaderPool::instance().get("gPass"));
        }
        mesh.addElement(elem);
    }

    return mesh;
}

void XmlMeshAssetLoader::parseMeshAssetElement(TiXmlElement* node)
{
    vector<MeshElementModel> meshModel;
    std::string name = str(node->Attribute("name"));

    if(name.empty())
        return;

    node=node->FirstChildElement();
    while(node)
    {
        if(node->ValueStr() == "Element")
        {
            int type = -1;
            node->QueryIntAttribute("type", &type);

            if(type == 0)
            {
                MeshElementModel elementModel;
                TiXmlElement* elem=node->FirstChildElement();

                while(elem)
                {
                    if(StringUtils(elem->ValueStr()).toLower().str() == "color")
                        elementModel.color = toColor(str(elem->GetText()));
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "geometry")
                        elementModel.geometry = str(elem->GetText());

                    else if(StringUtils(elem->ValueStr()).toLower().str() == "roughness")
                        elementModel.material[0] = StringUtils(str(elem->GetText())).toFloat();
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "metallic")
                        elementModel.material[1] = StringUtils(str(elem->GetText())).toFloat();
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "specular")
                        elementModel.material[2] = StringUtils(str(elem->GetText())).toFloat();
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "emissive")
                        elementModel.material[3] = StringUtils(str(elem->GetText())).toFloat();

                    else if(StringUtils(elem->ValueStr()).toLower().str() == "diffusetex")
                        elementModel.textures[0] = str(elem->GetText());
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "normaltex")
                        elementModel.textures[1] = str(elem->GetText());
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "materialtex")
                        elementModel.textures[2] = str(elem->GetText());

                    elem=elem->NextSiblingElement();
                }
                meshModel.push_back(elementModel);
            }
        }

        node=node->NextSiblingElement();
    }

    if(meshModel.size() > 0)
        _models[name] = meshModel;
}

vec3 XmlMeshAssetLoader::toColor(std::string str)
{
    vector<std::string> v = StringUtils(str).splitWord(',');
    vec3 res;

    for(uint i=0 ; i<std::min(v.size(), 3u) ; ++i)
        res[i] = StringUtils(v[i]).toFloat() / 255;

    return res;
}

std::string XmlMeshAssetLoader::str(const char* c)
{
    if(c) return std::string(c);
    else return "";
}

}
}

