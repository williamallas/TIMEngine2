#include "bullet/GeometryShape.h"

namespace tim
{

//btBvhTriangleMeshShape* GeometryShape::genStaticGeometryShape(const vector<MeshInstance>& geometry)
//{
//    uint nbVertex=0;
//    uint nbIndex=0;
//    for(size_t i=0 ; i<geometry.size() ; ++i)
//    {
//        if(geometry[i].mesh && geometry[i].mesh->primitive()==renderer::VertexMode::TRIANGLES)
//        {
//            nbVertex += geometry[i].mesh->vertexBuffer()->size();
//            nbIndex += geometry[i].mesh->indexBuffer()->size();
//        }
//    }

//    if(nbVertex==0 || nbIndex==0)
//        return nullptr;

//    vec3* vdata = new vec3[nbVertex];
//    int* idata = new int[nbIndex];

//    vec3* iterator_vdata = vdata;
//    int* iterator_idata = idata;

//    int indexOffset=0;

//    for(size_t i=0 ; i<geometry.size() ; ++i)
//    {
//        if(geometry[i].mesh && geometry[i].mesh->primitive()==renderer::VertexMode::TRIANGLES)
//        {
//            float* data = geometry[i].mesh->vertexBuffer()->data();
//            uint formatSize = geometry[i].mesh->vertexBuffer()->formatSize();

//            for(size_t j=0 ; j<geometry[i].mesh->vertexBuffer()->size() ; ++j)
//                *(iterator_vdata++) = geometry[i].mat * vec3(data[j*formatSize], data[j*formatSize+1], data[j*formatSize+2]);

//            for(size_t j=0 ; j<geometry[i].mesh->indexBuffer()->size() ; ++j)
//                *(iterator_idata++) = geometry[i].mesh->indexBuffer()->data()[j]+indexOffset;

//            indexOffset += geometry[i].mesh->vertexBuffer()->size();
//        }
//    }

//    btTriangleIndexVertexArray* indexVertexArrays = new btTriangleIndexVertexArray(nbIndex/3, idata, sizeof(int)*3,
//                                                                                   nbVertex,reinterpret_cast<float*>(vdata),sizeof(vec3));

//    return new btBvhTriangleMeshShape(indexVertexArrays,true);
//}

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
