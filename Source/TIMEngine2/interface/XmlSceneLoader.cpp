#include "XmlSceneLoader.h"
#include "ShaderPool.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace interface
{

bool XmlSceneLoader::loadScene(std::string file, interface::Scene& scene, vector<ObjectLoaded>& objects)
{
    TiXmlDocument doc(file);

    if(!doc.LoadFile())
        return false;

    TiXmlElement* root=doc.FirstChildElement();
    TiXmlElement* elem = root;

    // first parse meshasset
    std::map<int, vector<XmlMeshAssetLoader::MeshElementModel>> meshAssets;
    std::map<int, std::string> meshAssetsName;

    int nbObject=0;

    while(elem)
    {
        if(elem->ValueStr() == std::string("MeshAsset"))
        {
            int index=-1;
            elem->QueryIntAttribute("index", &index);

            std::string name;
            meshAssets[index] = interface::XmlMeshAssetLoader::parseMeshAssetElement(elem, name);
            meshAssetsName[index] = name;
        }
        else if(elem->ValueStr() == std::string("Object"))
        {
            nbObject++;
        }
        else if(elem->ValueStr() == std::string("DirLight"))
        {
            int shadow=0;
            vec3 color = {1,1,1};
            vec3 dir={0,0,-1};

            elem->QueryIntAttribute("shadows", &shadow);
            std::string strColor = StringUtils::str(elem->Attribute("color"));
            std::string strDir = StringUtils::str(elem->Attribute("direction"));
            if(!strColor.empty()) color = toVec<3>(strColor);
            if(!strDir.empty()) dir = toVec<3>(strDir);

            scene.globalLight.dirLights.push_back({dir, vec4(color,1), shadow==1});
        }

        elem=elem->NextSiblingElement();
    }

    elem = root;
    vector<std::string> skybox;

    while(elem)
    {
        if(elem->ValueStr() == std::string("Object"))
        {
            std::string name;
            elem->QueryStringAttribute("name", &name);

            int index=-1;
            elem->QueryIntAttribute("model", &index);

            if(index < 0 || meshAssets.find(index)==meshAssets.end())
                continue;

            bool isStatic = true, isPhysic = true;
            elem->QueryBoolAttribute("isStatic", &isStatic);
            elem->QueryBoolAttribute("isPhysic", &isPhysic);

            vec3 tr, sc;
            mat3 rot;
            parseTransformation(elem, tr, sc, rot);

            ObjectLoaded obj;
            obj.name = name;
            obj.model = meshAssetsName[index];
            obj.type = ObjectLoaded::MESH_INSTANCE;
            obj.asset = meshAssets[index];
            obj.isPhysic = isPhysic;
            obj.isStatic = isStatic;

            obj.meshInstance = &scene.scene.add<MeshInstance>(XmlMeshAssetLoader::constructMesh(meshAssets[index], Texture::genParam(true,true,true, 0), false),
                                                              mat4::constructTransformation(rot, tr, sc));
            objects.push_back(obj);
        }
        else if(elem->ValueStr() == std::string("Skybox"))
        {
            skybox = parseSkyboxXmlElement(elem);
        }

        elem=elem->NextSiblingElement();
    }

    if(skybox.size() == 6)
    {
        scene.globalLight.skybox = renderer::IndirectLightRenderer::loadAndProcessSkybox(skybox, ShaderPool::instance().get("processSpecularCubeMap"));
    }

    return true;
}

void XmlSceneLoader::parseTransformation(TiXmlElement* elem, vec3& tr, vec3& sc, mat3& rot)
{
    tr={0,0,0};
    sc={1,1,1};
    rot=mat3::IDENTITY();

    elem = elem->FirstChildElement();

    while(elem)
    {
        if(elem->ValueStr() == std::string("translate"))
            tr = toVec<3>(StringUtils::str(elem->GetText()));
        else if(elem->ValueStr() == std::string("scale"))
            sc = toVec<3>(StringUtils::str(elem->GetText()));
        else if(elem->ValueStr() == std::string("rotate"))
        {
            Vector<float, 9> r = toVec<9>(StringUtils::str(elem->GetText()));

            for(int i=0 ; i<9 ; ++i)
                rot.get(i) = r[i];
        }

        elem = elem->NextSiblingElement();
    }
}

vector<std::string> XmlSceneLoader::parseSkyboxXmlElement(TiXmlElement* elem)
{
    vector<std::string> res(6);

    elem = elem->FirstChildElement();

    while(elem)
    {
        if(elem->ValueStr() == std::string("x"))
            res[0] = StringUtils::str(elem->GetText());
        else if(elem->ValueStr() == std::string("nx"))
            res[1] = StringUtils::str(elem->GetText());
        else if(elem->ValueStr() == std::string("y"))
            res[2] = StringUtils::str(elem->GetText());
        else if(elem->ValueStr() == std::string("ny"))
            res[3] = StringUtils::str(elem->GetText());
        else if(elem->ValueStr() == std::string("z"))
            res[4] = StringUtils::str(elem->GetText());
        else if(elem->ValueStr() == std::string("nz"))
            res[5] = StringUtils::str(elem->GetText());

        elem = elem->NextSiblingElement();
    }

    return res;
}

}
}

