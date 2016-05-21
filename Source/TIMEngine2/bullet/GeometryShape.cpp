#include "bullet/GeometryShape.h"
#include "bullet/BulletEngine.h"

namespace tim
{

btBvhTriangleMeshShape* GeometryShape::genStaticGeometryShape(const vector<MeshInstance>& geometry)
{
    uint nbVertex=0;
    uint nbIndex=0;
    for(size_t i=0 ; i<geometry.size() ; ++i)
    {
        if(!geometry[i].mesh.isEmpty() && geometry[i].mesh.meshData())
        {
            nbVertex += geometry[i].mesh.meshData()->nbVertex;
            nbIndex += geometry[i].mesh.meshData()->nbIndex;
        }
    }

    if(nbVertex==0 || nbIndex==0)
        return nullptr;

    vec3* vdata = new vec3[nbVertex];
    int* idata = new int[nbIndex];

    vec3* iterator_vdata = vdata;
    int* iterator_idata = idata;

    int indexOffset=0;

    for(size_t i=0 ; i<geometry.size() ; ++i)
    {
        if(!geometry[i].mesh.isEmpty() && geometry[i].mesh.meshData())
        {
            const renderer::MeshData* meshData = geometry[i].mesh.meshData();
            //renderer::MeshData::DataType* data = meshData->vData;
            //uint formatSize = renderer::vertexFormatSize(renderer::MeshData::DATA_ID);

            for(size_t j=0 ; j<meshData->nbVertex ; ++j)
                *(iterator_vdata++) = geometry[i].mat * meshData->vData[j].v;

            for(size_t j=0 ; j<meshData->nbIndex ; ++j)
                *(iterator_idata++) = meshData->indexData[j]+indexOffset;

            indexOffset += meshData->nbVertex;
        }
    }

    btTriangleIndexVertexArray* indexVertexArrays = new btTriangleIndexVertexArray(nbIndex/3, idata, sizeof(int)*3,
                                                                                   nbVertex,reinterpret_cast<float*>(vdata),sizeof(vec3));

    return new btBvhTriangleMeshShape(indexVertexArrays,true);
}

//btBvhTriangleMeshShape* GeometryShape::genStaticGeometryShape(const resource::assimpMeshLoader& loader,
//                                                              const boost::container::set<std::string>& ignore)
//{
//    vector<MeshInstance> geom;
//    for(uint i=0 ; i<loader.nodes().size() ; ++i)
//    {
//        for(uint j=0 ; j<loader.nodes()[i].meshs.size() ; ++j)
//        {
//            renderer::MeshBuffers* m = loader.getMeshLoaded()[loader.nodes()[i].meshs[j]].mesh;
//            if(m && ignore.find(loader.nodes()[i].name)==ignore.end())
//                geom.push_back({m, loader.nodes()[i].matrix});
//        }
//    }
//    return genStaticGeometryShape(geom);
//}

}
