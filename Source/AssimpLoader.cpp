#include "AssimpLoader.h"

#include "core/core.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;

AssimpLoader::AssimpLoader()
{
    //ctor
}

AssimpLoader::~AssimpLoader()
{
    //dtor
}

bool AssimpLoader::load(const std::string& file)
{
    const aiScene* scene = importer.ReadFile(file,
        aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_ImproveCacheLocality   |
        aiProcess_SortByPType);

    if(!scene) return false;

    processSceneNode(scene->mRootNode);

    return true;
}

void AssimpLoader::processSceneNode(aiNode* node, mat4 m)
{
    uint i=0;
    std::string name=node->mName.C_Str();
    std::string idName;
    while(i<name.size() && name[i] != '.' && name[i] != '_')
    {
        idName += name[i];
        ++i;
    }

    if(node->mParent != nullptr)
        m *= convert(node->mTransformation);

    _nodes.push_back({name, idName, m.translated(-m.translation()*0.5f)});

    for(uint i=0 ; i<node->mNumChildren ; ++i)
    {
        processSceneNode(node->mChildren[i], m);
    }
}

vector<AssimpLoader::Node> AssimpLoader::findNodeWithIdName(std::string n) const
{
    vector<AssimpLoader::Node> res;
    for(uint i=0 ; i<_nodes.size() ; ++i)
    {
        if(_nodes[i].idName == n)
            res.push_back(_nodes[i]);
    }
    return res;
}

AssimpLoader::Node AssimpLoader::findNode(std::string n) const
{
    for(uint i=0 ; i<_nodes.size() ; ++i)
    {
        if(_nodes[i].name == n)
            return _nodes[i];
    }
    return Node();
}

mat4 AssimpLoader::convert(const aiMatrix4x4& m)
{
    return mat4({m.a1, m.a2, m.a3, m.a4,
                 m.b1, m.b2, m.b3, m.b4,
                 m.c1, m.c2, m.c3, m.c4,
                 m.d1, m.d2, m.d3, m.d4});
}

vec3 AssimpLoader::convert(const aiVector3D& v)
{
    return vec3(v.x, v.y, v.z);
}

vec4 AssimpLoader::convert(const aiQuaternion& q)
{
    return vec4(q.x, q.y, q.z, q.w);
}

//void assimpMeshLoader::makeNodeNameUnique()
//{
//    boost::container::map<std::string, uint> countName;
//    for(uint i=0 ; i<_nodes.size() ; ++i)
//        countName[_nodes[i].name]++;

//    for(auto name : countName)
//    {
//        if(name.second <= 1)
//            continue;

//        int counter=0;
//        for(uint j=0 ; j<_nodes.size() ; ++j)
//        {
//            if(_nodes[j].name == name.first)
//                _nodes[j].name = name.first+StringUtils(++counter).str();
//        }
//    }
//}


}
