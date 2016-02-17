#include "MeshLoader.h"
#include "core/core.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace resource
{

/****************/
/** OBJ loader **/
/****************/

MeshLoader::LoadedMeshData MeshLoader::importObj(const std::string& file, bool tangent)
{
    LoadedMeshData meshData;
    meshData.name = file;

    ObjBuffer buf;
    if(!loadObjData(file, buf))
        return meshData;

    boost::container::map<renderer::VNC_Vertex, size_t> mapIndex;

    size_t nbVertex = computeObjVertexMap(buf, meshData, mapIndex);

    if(buf.nbNormal && buf.nbTexCoord && tangent)
            meshData.format = renderer::VertexFormat::VNCT;
    else if(buf.nbNormal && buf.nbTexCoord)
            meshData.format = renderer::VertexFormat::VNC;
    else if(buf.nbNormal)
            meshData.format = renderer::VertexFormat::VN;
    else
            meshData.format = renderer::VertexFormat::V;

    meshData.nbVertex = nbVertex;
    meshData.vData = new renderer::VNCT_Vertex[nbVertex];
    for(auto it : mapIndex)
        meshData.vData[it.second] = {it.first.v, it.first.n, it.first.c, vec3()};

    if(meshData.format == renderer::VertexFormat::VNCT)
        computeTangent(meshData);

    buf.free();
    return meshData;
}

renderer::MeshBuffers* MeshLoader::createMeshBuffers(LoadedMeshData& data, renderer::VertexBufferPoolType* vpool, renderer::IndexBufferPoolType* ipool)
{
    if(data.nbIndex > 0 && data.nbVertex > 0)
    {
        renderer::VBuffer* vb = vpool->alloc(data.nbVertex);
        renderer::IBuffer* ib = ipool->alloc(data.nbIndex);
        vb->flush(reinterpret_cast<float*>(data.vData), 0, data.nbVertex);
        ib->flush(data.indexData, 0, data.nbIndex);
        return new renderer::MeshBuffers(vb, ib);
    }
    else return nullptr;

}

//MeshBuffers* MeshLoader::createMeshBuffers(const float* vdata, size_t nbv,
//                                                     const uint* idata, size_t nbi,
//                                                     VertexFormat format)
//{
//    size_t size;
//    switch(format)
//    {
//        case VertexFormat::V: size = nbv*3; break;
//        case VertexFormat::VC: size = nbv*5; break;
//        case VertexFormat::VN: size = nbv*6; break;
//        case VertexFormat::VNC: size = nbv*8; break;
//        case VertexFormat::VNCT: size = nbv*11; break;

//        case VertexFormat::VEC1: size = nbv*1; break;
//        case VertexFormat::VEC2: size = nbv*2; break;
//        case VertexFormat::VEC3: size = nbv*3; break;
//        case VertexFormat::VEC4: size = nbv*4; break;
//    }

//    VertexBuffer* vb = new VertexBuffer();
//    vb->createBuffer(format, nbv);
//    for(size_t i=0 ; i<size ; ++i)
//        vb->data()[i]=vdata[i];

//    IndexBuffer* ib = new IndexBuffer();

//    ib->createBuffer(nbi);
//    for(size_t i=0 ; i<nbi ; ++i)
//        ib->data()[i]=idata[i];

//    return new MeshBuffers({vb}, ib);
//}

bool MeshLoader::loadObjData(const std::string& file, ObjBuffer& buffer)
{
    {
        std::ifstream readNbV(file);
        buffer.nbVertex=0;
        buffer.nbNormal=0;
        buffer.nbTexCoord=0;
        buffer.nbIndex=0;
        std::string s;
        while(readNbV.good())
        {
            readNbV >> s;
            if(s=="v")
                ++buffer.nbVertex;
            else if(s=="vn")
                ++buffer.nbNormal;
            else if(s=="vt")
                ++buffer.nbTexCoord;
            else if(s=="f")
                buffer.nbIndex+=3;
            else if(s=="l")
                buffer.nbIndex+=2;
        }

        if(!buffer.nbVertex || !buffer.nbIndex)
            return false;
    }

    int nbSlash=2;
    if(buffer.nbNormal == 0 && buffer.nbTexCoord == 0)
        nbSlash=0;
    else if(buffer.nbNormal == 0)
        nbSlash=1;

    buffer.vbuffer = new vec3[buffer.nbVertex];
    buffer.ibuffer = new uivec3[buffer.nbIndex];

    if(buffer.nbNormal)
        buffer.nbuffer = new vec3[buffer.nbNormal];

    if(buffer.nbTexCoord)
        buffer.tbuffer = new vec2[buffer.nbTexCoord];

    std::ifstream sfile(file);
    std::string str;

    size_t vindex=0, nindex=0, tindex=0, iindex=0;

    while(sfile.good())
    {
        sfile >> str;

        if(str=="v" || str=="vn")
        {
            std::string xyz[3];
            vec3 v;
            for(size_t i=0 ; i<3 ; ++i)
            {
                sfile >> xyz[i];
                if(!StringUtils(xyz[i]).isNumber())
                {

                    buffer.free();
                    return false;
                }
                else v[i] = StringUtils(xyz[i]).toFloat();
            }

            if(str=="v")
            {
                buffer.vbuffer[vindex] = v;
                ++vindex;
            }
            else
            {
                buffer.nbuffer[nindex] = v;
                ++nindex;
            }
        }
        else if(str=="vt")
        {
            std::string str;
            for(size_t i=0 ; i<2 ; ++i)
            {
                sfile >> str;
                if(!StringUtils(str).isNumber())
                {
                    buffer.free();
                    return false;
                }
                else
                    buffer.tbuffer[tindex][i] = StringUtils(str).toFloat();
            }
            ++tindex;
        }
        else if(str=="f")
        {
            for(size_t i=0 ; i<3 ; ++i)
            {
                sfile >> str;
                bool b=true;
                buffer.ibuffer[iindex] = parseObjIndex(str, b, nbSlash);
                if(!b || buffer.ibuffer[iindex].x()>buffer.nbVertex || (buffer.ibuffer[iindex].x()==0 && buffer.nbVertex)
                      || buffer.ibuffer[iindex].z()>buffer.nbNormal || (buffer.ibuffer[iindex].z()==0 && buffer.nbNormal)
                      || buffer.ibuffer[iindex].y()>buffer.nbTexCoord || (buffer.ibuffer[iindex].y()==0 && buffer.nbTexCoord))
                {
                    buffer.free();
                    return false;
                }
                ++iindex;
            }
        }
        else if(str=="l")
        {
            for(size_t i=0 ; i<2 ; ++i)
            {
                sfile >> str;
                bool b=true;
                buffer.ibuffer[iindex] = parseObjIndex(str, b, nbSlash);
                if(!b || buffer.ibuffer[iindex].x()>buffer.nbVertex || (buffer.ibuffer[iindex].x()==0 && buffer.nbVertex)
                      || buffer.ibuffer[iindex].z()>buffer.nbNormal || (buffer.ibuffer[iindex].z()==0 && buffer.nbNormal)
                      || buffer.ibuffer[iindex].y()>buffer.nbTexCoord || (buffer.ibuffer[iindex].y()==0 && buffer.nbTexCoord))
                {
                    buffer.free();
                    return false;
                }
                ++iindex;
            }
        }
    }

    if(iindex != buffer.nbIndex || nindex != buffer.nbNormal ||
       vindex != buffer.nbVertex || tindex != buffer.nbTexCoord)
       {
           buffer.free();
           return false;
       }


    return true;
}

size_t MeshLoader::computeObjVertexMap(ObjBuffer& buf, LoadedMeshData& meshData,
                                       boost::container::map<renderer::VNC_Vertex, size_t>& mapIndex)
{
    size_t curIndex=0;
    meshData.nbIndex = buf.nbIndex;
    meshData.indexData = new uint[buf.nbIndex];

    renderer::VNC_Vertex vnc;
    for(size_t i=0 ; i<buf.nbIndex ; ++i)
    {
        vnc.v = buf.vbuffer[buf.ibuffer[i].x()-1];
        vnc.n=vec3(); vnc.c=vec2();
        if(buf.nbNormal)
            vnc.n = buf.nbuffer[buf.ibuffer[i].z()-1];
        if(buf.nbTexCoord)
            vnc.c = buf.tbuffer[buf.ibuffer[i].y()-1];

        auto it = mapIndex.find(vnc);
        if(it == mapIndex.end())
        {
            mapIndex[vnc] = curIndex;
            meshData.indexData[i] = curIndex;
            ++curIndex;
        }
        else
        {
            meshData.indexData[i] = it->second;
        }
    }

    return curIndex;
}

uivec3 MeshLoader::parseObjIndex(const std::string& str, bool& ok, int nbSlash)
{
    if(std::count(str.begin(), str.end(), '/') != nbSlash)
    {
        ok=false;
        return uivec3();
    }

    uivec3 res;

    std::string buf;
    size_t index=0;

    for(size_t i=0 ; i<3 ; ++i)
    {
        while(index < str.size() && str[index] != '/')
        {
            buf+=str[index];
            ++index;
        }
        if(!buf.empty())
        {
            if(!StringUtils(buf).isNumber())
            {
                ok=false;
                return uivec3();
            }
            res[i] = StringUtils(buf).toInt();
        }
        buf.clear(); ++index;
    }
    return res;
}

void MeshLoader::computeTangent(LoadedMeshData& meshData)
{
    if(meshData.format != renderer::VertexFormat::VNCT || meshData.indexData == nullptr || meshData.vData == nullptr)
        return;

    uivec3* triangles = reinterpret_cast<uivec3*>(meshData.indexData);
    uint trianglesCount = meshData.nbIndex/3;

    renderer::VNCT_Vertex* vbuffer = meshData.vData;
    uint vertexCount = meshData.nbVertex;

    vec3* tan1 = new vec3[vertexCount*2]();
    vec3* tan2 = tan1+vertexCount;

    /** http://www.terathon.com/code/tangent.html */
    for(uint i=0 ; i<trianglesCount ; ++i)
    {
        uint i1=triangles[i].x();
        uint i2=triangles[i].y();
        uint i3=triangles[i].z();

        const vec3& v1 = vbuffer[i1].v;
        const vec3& v2 = vbuffer[i2].v;
        const vec3& v3 = vbuffer[i3].v;

        const vec2& w1 = vbuffer[i1].c;
        const vec2& w2 = vbuffer[i2].c;
        const vec2& w3 = vbuffer[i3].c;

        float x1 = v2.x() - v1.x();
        float x2 = v3.x() - v1.x();
        float y1 = v2.y() - v1.y();
        float y2 = v3.y() - v1.y();
        float z1 = v2.z() - v1.z();
        float z2 = v3.z() - v1.z();

        float s1 = w2.x() - w1.x();
        float s2 = w3.x() - w1.x();
        float t1 = w2.y() - w1.y();
        float t2 = w3.y() - w1.y();

        float r = 1.0f / (s1 * t2 - s2 * t1);
        vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                (t2 * z1 - t1 * z2) * r);
        vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                (s1 * z2 - s2 * z1) * r);

        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;

        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
    }

    for (uint i=0 ; i<vertexCount ; ++i)
    {
        const vec3& n = vbuffer[i].n;
        const vec3& t = tan1[i];

        // Gram-Schmidt orthogonalize
        vbuffer[i].t = (t - n * n.dot(t)).normalize();

//        // Calculate handedness
//        tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
    }

    delete[] tan1;
}

///****************/
///** TIM loader **/
///****************/


//MeshBuffers* MeshLoader::importMeshTim(const std::string& file)
//{
//    std::ifstream ff(file, std::ios_base::binary);
//    if(!ff)
//        return nullptr;

//    return readMeshTim(ff);
//}

//MeshBuffers* MeshLoader::readMeshTim(std::istream& ff)
//{
//    byte header[4] = {0,0,0,0};
//    ff.read(header, 4);
//    if(header[0] != 1 || header[1] != 0 || header[2] != 0 || header[3] != 0)
//        return nullptr;

//    uint primitive;
//    ff.read(reinterpret_cast<char*>(&primitive), sizeof(uint));

//    uint nbIBuf=0, nbVBuf=0;

//    read(ff, nbVBuf);
//    read(ff, nbIBuf);
//    VertexBuffer* vbuf=nullptr;
//    IndexBuffer* ibuf=nullptr;

//    if(nbVBuf==1)
//    {
//        vbuf = new VertexBuffer();
//        for(uint i=0 ; i<nbVBuf ; i++)
//        {
//            uint size, format, formatSize;
//            read(ff, size);
//            read(ff, format);
//            read(ff, formatSize);
//            vbuf->createBuffer(static_cast<VertexFormat>(format), size);

//            ff.read(reinterpret_cast<char*>(vbuf->data()),
//                    size*formatSize*sizeof(real));
//        }
//    }

//    if(nbIBuf==1)
//    {
//        ibuf = new IndexBuffer();

//        for(uint i=0 ; i<nbIBuf ; i++)
//        {
//            uint size;
//            read(ff, size);
//            ibuf->createBuffer(size);

//            ff.read(reinterpret_cast<char*>(ibuf->data()),
//                    size*sizeof(uint));
//        }
//    }

//    uint nbAuxBuf=0;
//    read(ff, nbAuxBuf);
//    vector<GenericVertexBuffer<float>*> auxBuf(nbAuxBuf);
//    for(uint i=0 ; i<nbAuxBuf ; ++i)
//    {
//        uint size, format, formatSize;
//        read(ff, size);
//        read(ff, format);
//        read(ff, formatSize);

//        auxBuf[i] = new GenericVertexBuffer<float>;
//        auxBuf[i]->createBuffer(static_cast<VertexFormat>(format), size);

//        ff.read(reinterpret_cast<char*>(auxBuf[i]->data()),
//                size*formatSize*sizeof(real));
//    }

//    uint nbAuxBufi=0;
//    read(ff, nbAuxBufi);
//    vector<GenericVertexBuffer<int>*> auxBufi(nbAuxBufi);
//    for(uint i=0 ; i<nbAuxBufi ; ++i)
//    {
//        uint size, format, formatSize;
//        read(ff, size);
//        read(ff, format);
//        read(ff, formatSize);

//        auxBufi[i] = new GenericVertexBuffer<int>;
//        auxBufi[i]->createBuffer(static_cast<VertexFormat>(format), size);

//        ff.read(reinterpret_cast<char*>(auxBufi[i]->data()),
//                size*formatSize*sizeof(integer));
//    }

//    MeshBuffers* mesh = new MeshBuffers({vbuf}, ibuf, auxBuf, auxBufi);
//    mesh->setPrimitive(static_cast<VertexMode>(primitive));
//    return mesh;
//}

//void MeshLoader::exportTim(MeshBuffers* mesh, const std::string& file)
//{
//    std::ofstream ff(file, std::ios_base::binary);
//    if(!ff || !mesh)
//        return;

//    writeTim(ff, mesh);
//}

//void MeshLoader::writeTim(std::ostream& ff, MeshBuffers* mesh)
//{
//    byte header[4] = {1,0,0,0};
//    ff.write(header, 4);

//    write(ff, static_cast<uint>(mesh->primitive()));

//    write(ff, static_cast<uint>(mesh->vertexBuffer()!=nullptr?1:0));
//    write(ff, static_cast<uint>(mesh->indexBuffer()!=nullptr?1:0));

//    if(mesh->vertexBuffer())
//    {
//        write(ff, static_cast<uint>(mesh->vertexBuffer()->size()));
//        write(ff, static_cast<uint>(mesh->vertexBuffer()->format()));
//        write(ff, static_cast<uint>(mesh->vertexBuffer()->formatSize()));

//        ff.write(reinterpret_cast<const char*>(mesh->vertexBuffer()->data()),
//                 mesh->vertexBuffer()->size()*mesh->vertexBuffer()->formatSize()*sizeof(real));
//    }

//    if(mesh->indexBuffer())
//    {
//        write(ff, static_cast<uint>(mesh->indexBuffer()->size()));
//        ff.write(reinterpret_cast<const char*>(mesh->indexBuffer()->data()),
//                 mesh->indexBuffer()->size()*sizeof(uint));
//    }

//    write(ff, static_cast<uint>(mesh->auxBuffers().size()));
//    for(uint i=0 ; i<mesh->auxBuffers().size() ; ++i)
//    {
//        write(ff, static_cast<uint>(mesh->auxBuffers()[i]->size()));
//        write(ff, static_cast<uint>(mesh->auxBuffers()[i]->format()));
//        write(ff, static_cast<uint>(mesh->auxBuffers()[i]->formatSize()));

//        ff.write(reinterpret_cast<const char*>(mesh->auxBuffers()[i]->data()),
//                 mesh->auxBuffers()[i]->size()*mesh->auxBuffers()[i]->formatSize()*sizeof(real));
//    }

//    write(ff, static_cast<uint>(mesh->auxBuffersi().size()));
//    for(uint i=0 ; i<mesh->auxBuffersi().size() ; ++i)
//    {
//        write(ff, static_cast<uint>(mesh->auxBuffersi()[i]->size()));
//        write(ff, static_cast<uint>(mesh->auxBuffersi()[i]->format()));
//        write(ff, static_cast<uint>(mesh->auxBuffersi()[i]->formatSize()));

//        ff.write(reinterpret_cast<const char*>(mesh->auxBuffersi()[i]->data()),
//                 mesh->auxBuffersi()[i]->size()*mesh->auxBuffersi()[i]->formatSize()*sizeof(integer));
//    }
//}

//MeshBuffers* MeshLoader::genGrid(const vec2& size, uint res, float virtualZ)
//{
//    VertexBuffer* vb=new VertexBuffer();

//    {
//        vec3* buffer = reinterpret_cast<vec3*>(vb->createBuffer(V, res*res));
//        vec2 vsize=size/(res-1);

//        for(uint x=0 ; x<res ; x++)
//        {
//            for(uint y=0 ; y<res ; y++)
//            {
//                buffer[x*res+y] = vec3(x*vsize.x(), y*vsize.y(), 0)-vec3(size)*0.5;
//            }
//        }

//        vb->uploadOnGpu(true, true);
//    }

//    IndexBuffer* ib = new IndexBuffer();
//    {
//        uint* buffer = ib->createBuffer((res-1)*(res-1)*6);

//        for(uint x=0 ; x<res-1 ; x++)
//        {
//            for(uint y=0 ; y<res-1 ; y++)
//            {
//                uint index=x*(res-1)+y;
//                buffer[index*6+0] = (x+1)*res+y;
//                buffer[index*6+1] = x*res+y+1;
//                buffer[index*6+2] = x*res+y;

//                buffer[index*6+3] = (x+1)*res+y;
//                buffer[index*6+4] = (x+1)*res+y+1;
//                buffer[index*6+5] = x*res+y+1;
//            }
//        }

//        ib->uploadOnGpu(true,true);
//    }

//    MeshBuffers* mb=new MeshBuffers(vb,ib);
//    Box b=mb->vertexBuffer()->box();
//    b.setZ({0, virtualZ});
//    mb->vertexBuffer()->setBox(b);
//    return mb;
//}

//MeshBuffers* MeshLoader::genGrid(const vec3& size, Image* hm, VertexFormat vformat)
//{
//    if(!hm || hm->empty() || hm->size().x() != hm->size().y())
//        return nullptr;

//    uint res = hm->size().x();
//    VertexBuffer* vb=new VertexBuffer();

//    {
//        float* buffer = vb->createBuffer(vformat, res*res);
//        vec2 vsize=vec2(size)/(res-1);

//        for(uint x=0 ; x<res ; ++x)
//        {
//            for(uint y=0 ; y<res ; ++y)
//            {
//                uint index=0;
//                float h = hm->pixel(uivec2(x,y)).x() * size.z();
//                buffer[vertexFormatSize(vformat) * (x*res+y) +index++] = x*vsize.x()-size.x()*0.5f;
//                buffer[vertexFormatSize(vformat) * (x*res+y) +index++] = y*vsize.y()-size.y()*0.5f;
//                buffer[vertexFormatSize(vformat) * (x*res+y) +index++] = h;

//                if(vformat == VertexFormat::VN || vformat == VertexFormat::VNC ||
//                   vformat == VertexFormat::VNCT)
//                {
//                    //float z=hm->pixel(uivec2(x,y)).x();
//                    float down = hm->pixel(uivec2(x,std::min(y+1, res-1))).x();
//                    float up = (y==0 ? hm->pixel(uivec2(x,0)).x() : hm->pixel(uivec2(x,y-1)).x());
//                    float left = (x==0 ? hm->pixel(uivec2(0, y)).x() : hm->pixel(uivec2(x-1, y)).x());
//                    float right = hm->pixel(uivec2(std::min(x+1, res-1),y)).x();

//                    vec3 n = vec3(right-left,up-down, size.x()/(res*size.z())).normalized();
//                    buffer[vertexFormatSize(vformat) * (x*res+y) +index++] = n.x();
//                    buffer[vertexFormatSize(vformat) * (x*res+y) +index++] = n.y();
//                    buffer[vertexFormatSize(vformat) * (x*res+y) +index++] = n.z();
//                }

//                if(vformat == VertexFormat::VC || vformat == VertexFormat::VNC ||
//                   vformat == VertexFormat::VNCT)
//                {
//                    buffer[vertexFormatSize(vformat) * (x*res+y) +index++] = static_cast<float>(x) / (res-1);
//                    buffer[vertexFormatSize(vformat) * (x*res+y) +index++] = static_cast<float>(y) / (res-1);
//                }
//            }
//        }

//        vb->uploadOnGpu(true, true);
//    }

//    IndexBuffer* ib = new IndexBuffer();
//    {
//        uint* buffer = ib->createBuffer((res-1)*(res-1)*6);

//        for(uint x=0 ; x<res-1 ; ++x)
//        {
//            for(uint y=0 ; y<res-1 ; ++y)
//            {
//                uint index=x*(res-1)+y;
//                buffer[index*6+0] = (x+1)*res+y;
//                buffer[index*6+1] = x*res+y+1;
//                buffer[index*6+2] = x*res+y;

//                buffer[index*6+3] = (x+1)*res+y;
//                buffer[index*6+4] = (x+1)*res+y+1;
//                buffer[index*6+5] = x*res+y+1;
//            }
//        }

//        ib->uploadOnGpu(true,true);
//    }

//    MeshBuffers* mb=new MeshBuffers(vb,ib);
//    Box b=mb->vertexBuffer()->box();
//    mb->vertexBuffer()->setBox(b);
//    return mb;
//}

//void MeshLoader::exportSkeleton(Skeleton* sk, const vector<SkeletonAnimation*>& anims, const std::string& file)
//{
//    std::ofstream ff(file, std::ios_base::binary);
//    if(!ff || !sk || !sk->root())
//        return;

//    byte header[4] = {54,87,37,64};
//    ff.write(header, 4);

//    write(ff, static_cast<uint>(sk->nbBones()));
//    for(size_t i=0 ; i<sk->nbBones() ; ++i)
//    {
//        Skeleton::Bone* bone = sk->bone(i);
//        write(ff, bone->name);
//        write(ff, bone->offset);
//        write(ff, bone->trans);
//        write(ff, bone->uniformIndex);

//        if(bone->parent)
//            write(ff, static_cast<uint>(bone->parent->index));
//        else
//            write(ff, static_cast<uint>(sk->nbBones()));

//        write(ff, static_cast<uint>(bone->childs.size()));
//        for(uint j=0 ; j<bone->childs.size() ; ++j)
//            write(ff, static_cast<uint>(bone->childs[j]->index));
//    }

//    write(ff, static_cast<uint>(anims.size()));
//    for(size_t i=0 ; i<anims.size() ; ++i)
//    {
//        if(anims[i]->name().empty()) write(ff, StringUtils(i+1).str());
//        else write(ff, anims[i]->name());
//        write(ff, anims[i]->_duration);
//        write(ff, anims[i]->_bonesCoordinated);
//        write(ff, anims[i]->_repeat);
//        write(ff, anims[i]->_roundTrip);
//        write(ff, anims[i]->_bones.size());
//        for(uint j=0 ; j<anims[i]->_bones.size() ; ++j)
//        {
//            write(ff, anims[i]->_bones[j]);
//            write(ff, static_cast<uint>(anims[i]->_scaleKeys[j].size()));
//            for(uint k=0 ; k<anims[i]->_scaleKeys[j].size() ; ++k)
//                write(ff, anims[i]->_scaleKeys[j][k]);

//            write(ff, static_cast<uint>(anims[i]->_posKeys[j].size()));
//            for(uint k=0 ; k<anims[i]->_posKeys[j].size() ; ++k)
//                write(ff, anims[i]->_posKeys[j][k]);

//            write(ff, static_cast<uint>(anims[i]->_rotKeys[j].size()));
//            for(uint k=0 ; k<anims[i]->_rotKeys[j].size() ; ++k)
//                write(ff, anims[i]->_rotKeys[j][k]);
//        }
//    }
//}

//Skeleton* MeshLoader::importSkeleton(const std::string& file, vector<SkeletonAnimation*>& anims)
//{
//    std::ifstream ff(file, std::ios_base::binary);
//    if(!ff)
//        return nullptr;

//    byte header[4] = {0,0,0,0};
//    ff.read(header, 4);
//    if(header[0] != 54 || header[1] != 87 || header[2] != 37 || header[3] != 64)
//        return nullptr;

//    uint nbBones;
//    read(ff, nbBones);
//    vector<Skeleton::Bone*> allBones(nbBones);
//    vector<vector<uint>> bonesChilds(nbBones);
//    vector<uint> bonesParent(nbBones);

//    for(size_t i=0 ; i<nbBones ; ++i)
//    {
//        allBones[i] = new Skeleton::Bone;
//        read(ff, allBones[i]->name);
//        read(ff, allBones[i]->offset);
//        read(ff, allBones[i]->trans);
//        read(ff, allBones[i]->uniformIndex);
//        read(ff, bonesParent[i]);

//        uint nbChilds=0;
//        read(ff, nbChilds);
//        bonesChilds[i].resize(nbChilds);
//        for(uint j=0 ; j<nbChilds ; ++j)
//            read(ff, bonesChilds[i][j]);
//    }

//    Skeleton::Bone* root = nullptr;
//    for(size_t i=0 ; i<nbBones ; ++i)
//    {
//        allBones[i]->index = i;
//        allBones[i]->childs.resize(bonesChilds[i].size());
//        for(uint j=0 ; j<bonesChilds[i].size() ; ++j)
//            allBones[i]->childs[j] = allBones[bonesChilds[i][j]];
//        if(bonesParent[i] == nbBones)
//        {
//            allBones[i]->parent = nullptr;
//            root = allBones[i];
//        }
//        else allBones[i]->parent = allBones[bonesParent[i]];
//    }

//    uint nbAnim=0;
//    read(ff, nbAnim);
//    for(uint i=0 ; i<nbAnim ; ++i)
//        anims.push_back(readSkeletonAnimation(ff));

//    return new Skeleton(root);
//}

//SkeletonAnimation* MeshLoader::readSkeletonAnimation(std::istream& in)
//{
//    SkeletonAnimation* anim = new SkeletonAnimation;

//    read(in, anim->_name);
//    read(in, anim->_duration);
//    read(in, anim->_bonesCoordinated);
//    read(in, anim->_repeat);
//    read(in, anim->_roundTrip);

//    uint nbBones;
//    read(in, nbBones);

//    anim->_bones.resize(nbBones);
//    anim->_posKeys = new vector<SkeletonAnimation::PositionKey>[nbBones];
//    anim->_scaleKeys = new vector<SkeletonAnimation::ScalingKey>[nbBones];
//    anim->_rotKeys = new vector<SkeletonAnimation::RotationKey>[nbBones];

//    for(uint j=0 ; j<nbBones ; ++j)
//    {
//        read(in, anim->_bones[j]);
//        anim->_bonesId[anim->_bones[j]] = j;
//        uint nbKeys;

//        read(in, nbKeys);
//        anim->_scaleKeys[j].resize(nbKeys);
//        for(uint k=0 ; k<nbKeys ; ++k)
//            read(in, anim->_scaleKeys[j][k]);

//        read(in, nbKeys);
//        anim->_posKeys[j].resize(nbKeys);
//        for(uint k=0 ; k<nbKeys ; ++k)
//            read(in, anim->_posKeys[j][k]);

//        read(in, nbKeys);
//        anim->_rotKeys[j].resize(nbKeys);
//        for(uint k=0 ; k<nbKeys ; ++k)
//            read(in, anim->_rotKeys[j][k]);
//    }

//    return anim;
//}

void MeshLoader::write(std::ostream& os, const std::string& str)
{
    write(os, static_cast<uint>(str.size()));
    if(str.size() > 0)
        os.write(reinterpret_cast<const char*>(str.c_str()), str.size());
}

void MeshLoader::read(std::istream& os, std::string& str)
{
    uint size;
    read(os, size);

    if(size > 0)
    {
        char* dat = new char[size+1];
        os.read(dat, size);
        dat[size] = '\0';
        str = dat;
        delete[] dat;
    }
    else str = "";
}

}
}
