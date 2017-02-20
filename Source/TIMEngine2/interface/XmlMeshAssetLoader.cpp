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
        {
            std::string name;
            auto asset = parseMeshAssetElement(elem, name);
            if(!name.empty() && !asset.empty())
                _models[name] = std::move(asset);
        }

        elem=elem->NextSiblingElement();
    }
    return true;
}

void XmlMeshAssetLoader::addModel(std::string name, const vector<MeshElementModel>& model)
{
    _models[name] = model;
}

Mesh XmlMeshAssetLoader::getMesh(std::string name, const renderer::Texture::GenTexParam& texParam) const
{
    auto it = _models.find(name);
    if(it == _models.end())
        return interface::Mesh();

    vector<MeshElementModel> model = it->second;

    return constructMesh(model, texParam);
}

interface::Mesh XmlMeshAssetLoader::constructMesh(const vector<MeshElementModel>& model, const renderer::Texture::GenTexParam& texParam, bool loadMeshMode)
{
    Mesh mesh;
    for(uint i=0 ; i<model.size() ; ++i)
    {
        Mesh::Element elem;
        if(model[i].type==0)
        {
            elem.setColor(vec4(model[i].color,1));
            elem.setRoughness(model[i].material[0]);
            elem.setMetallic(model[i].material[1]);
            elem.setSpecular(model[i].material[2]);
            elem.setEmissive(model[i].material[3]);
            elem.setTextureScale(model[i].textureScale);

            if(!model[i].geometry.empty())
                elem.setGeometry(resource::AssetManager<Geometry>::instance().load<false>(model[i].geometry, loadMeshMode).value());

            for(int j=0 ; j<3 ; ++j)
            {
                if(!model[i].textures[j].empty())
                    elem.setTexture(resource::AssetManager<Texture>::instance().load<false>(model[i].textures[j], texParam).value(), j);
            }

            if(!model[i].useAdvanced)
                elem.drawState().setShader(interface::ShaderPool::instance().get("gPass"));
            else
            {
                elem.drawState() = model[i].advanced;
                if(model[i].advancedShader.empty())
                    elem.drawState().setShader(interface::ShaderPool::instance().get("gPass"));
                else
                    elem.drawState().setShader(interface::ShaderPool::instance().get(model[i].advancedShader));
            }
        }
        elem.setCastShadow(model[i].castShadow);
        elem.setCubemapAffected(model[i].cmAffected);
        mesh.addElement(elem);
    }

    return mesh;
}

vector<XmlMeshAssetLoader::MeshElementModel> XmlMeshAssetLoader::parseMeshAssetElement(TiXmlElement* node, std::string& name)
{
    vector<MeshElementModel> meshModel;
    name = StringUtils::str(node->Attribute("name"));

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
                        elementModel.color = toColor(StringUtils::str(elem->GetText()));
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "geometry")
                        elementModel.geometry = StringUtils::str(elem->GetText());

                    else if(StringUtils(elem->ValueStr()).toLower().str() == "roughness")
                        elementModel.material[0] = StringUtils(StringUtils::str(elem->GetText())).toFloat();
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "metallic")
                        elementModel.material[1] = StringUtils(StringUtils::str(elem->GetText())).toFloat();
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "specular")
                        elementModel.material[2] = StringUtils(StringUtils::str(elem->GetText())).toFloat();
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "emissive")
                        elementModel.material[3] = StringUtils(StringUtils::str(elem->GetText())).toFloat();
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "texturescale")
                        elementModel.textureScale = StringUtils(StringUtils::str(elem->GetText())).toFloat();

                    else if(StringUtils(elem->ValueStr()).toLower().str() == "diffusetex")
                        elementModel.textures[0] = StringUtils::str(elem->GetText());
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "normaltex")
                        elementModel.textures[1] = StringUtils::str(elem->GetText());
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "materialtex")
                        elementModel.textures[2] = StringUtils::str(elem->GetText());
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "castshadow")
                        elementModel.castShadow = StringUtils(elem->GetText()).toBool();
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "cmaffected")
                        elementModel.cmAffected = StringUtils(elem->GetText()).toBool();
                    else if(StringUtils(elem->ValueStr()).toLower().str() == "advanced")
                    {
                        TiXmlElement* e=elem->FirstChildElement();
                        elementModel.useAdvanced = true;
                        while(e)
                        {
                            if(StringUtils(e->ValueStr()).toLower().str() == "blend")
                                elementModel.advanced.setBlend(StringUtils(StringUtils::str(e->GetText())).toBool());
                            else if(StringUtils(e->ValueStr()).toLower().str() == "cullbackface")
                                elementModel.advanced.setCullBackFace(StringUtils(StringUtils::str(e->GetText())).toBool());
                            else if(StringUtils(e->ValueStr()).toLower().str() == "cullface")
                                elementModel.advanced.setCullFace(StringUtils(StringUtils::str(e->GetText())).toBool());
                            else if(StringUtils(e->ValueStr()).toLower().str() == "depthtest")
                                elementModel.advanced.setDepthTest(StringUtils(StringUtils::str(e->GetText())).toBool());
                            else if(StringUtils(e->ValueStr()).toLower().str() == "writedepth")
                                elementModel.advanced.setWriteDepth(StringUtils(StringUtils::str(e->GetText())).toBool());
                            else if(StringUtils(e->ValueStr()).toLower().str() == "priority")
                                elementModel.advanced.setPriority(StringUtils(StringUtils::str(e->GetText())).toInt());
                            else if(StringUtils(e->ValueStr()).toLower().str() == "shader")
                                elementModel.advancedShader = StringUtils::str(e->GetText());
                            else if(StringUtils(e->ValueStr()).toLower().str() == "blendequation")
                                elementModel.advanced.setBlendEquation(renderer::DrawState::fromBlendEquationStr( StringUtils::str(e->GetText()) ));
                            else if(StringUtils(e->ValueStr()).toLower().str() == "primitive")
                                elementModel.advanced.setPrimitive(renderer::DrawState::fromPrimitiveStr( StringUtils::str(e->GetText()) ));
                            else if(StringUtils(e->ValueStr()).toLower().str() == "depthfunc")
                                elementModel.advanced.setDepthFunc(renderer::DrawState::fromComparFuncStr( StringUtils::str(e->GetText()) ));

                            else if(StringUtils(e->ValueStr()).toLower().str() == "blendfunc")
                            {
                                Vector2<renderer::DrawState::BlendFunc> funcs;
                                funcs[0] = renderer::DrawState::fromBlendFuncStr( StringUtils::str(e->Attribute("f1")) );
                                funcs[1] = renderer::DrawState::fromBlendFuncStr( StringUtils::str(e->Attribute("f2")) );
                                elementModel.advanced.setBlendFunc(funcs);
                            }

                            e = e->NextSiblingElement();
                        }
                    }

                    elem=elem->NextSiblingElement();
                }
                meshModel.push_back(elementModel);
            }
        }

        node=node->NextSiblingElement();
    }

    return meshModel;
}

vec3 XmlMeshAssetLoader::toColor(std::string str)
{
    vector<std::string> v = StringUtils(str).splitWord(',');
    vec3 res;

    for(uint i=0 ; i<std::min(v.size(), 3u) ; ++i)
        res[i] = StringUtils(v[i]).toFloat() / 255;

    return res;
}

}
}

