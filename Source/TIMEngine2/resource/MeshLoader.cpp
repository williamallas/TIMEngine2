#include "MeshLoader.h"
#include "core/core.h"
#include "Sphere.h"
#include <fstream>

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace resource
{

/****************/
/** OBJ loader **/
/****************/

renderer::MeshData MeshLoader::importObj(const std::string& file, bool tangent)
{
    renderer::MeshData meshData;
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
    meshData.vData = new renderer::MeshData::DataType[nbVertex];
	for (auto it : mapIndex)
	{
		meshData.vData[it.second] = { it.first.v, it.first.n, it.first.c, vec3() };
	}

    if(meshData.format == renderer::VertexFormat::VNCT)
        computeTangent(meshData);

    buf.free();
    return meshData;
}

renderer::MeshBuffers* MeshLoader::createMeshBuffers(renderer::MeshData& data, renderer::VertexBufferPoolType* vpool, renderer::IndexBufferPoolType* ipool)
{
    if(data.nbIndex > 0 && data.nbVertex > 0)
    {
        renderer::VBuffer* vb = vpool->alloc(data.nbVertex);
        renderer::IBuffer* ib = ipool->alloc(data.nbIndex);
        vb->flush(reinterpret_cast<float*>(data.vData), 0, data.nbVertex);
        ib->flush(data.indexData, 0, data.nbIndex);
        return new renderer::MeshBuffers(vb, ib, Sphere::computeSphere(reinterpret_cast<real*>(data.vData), data.nbVertex,
                                                                       sizeof(renderer::MeshData::DataType)/sizeof(float)));
    }
    else return nullptr;

}

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

size_t MeshLoader::computeObjVertexMap(ObjBuffer& buf, renderer::MeshData& meshData,
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

void MeshLoader::computeTangent(renderer::MeshData& meshData)
{
    if(meshData.format != renderer::VertexFormat::VNCT || meshData.indexData == nullptr || meshData.vData == nullptr)
        return;

    uivec3* triangles = reinterpret_cast<uivec3*>(meshData.indexData);
    uint trianglesCount = meshData.nbIndex/3;

    renderer::MeshData::DataType* vbuffer = meshData.vData;
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

renderer::MeshData MeshLoader::importTim(const std::string& file)
{
    renderer::MeshData data;

    std::ifstream fs(file, std::ios_base::binary);
    if(!fs)
        return data;

    char header[4] = {0,0,0,0};
    fs.read(header,4);
    if(!(header[0] == 43 && header[1] == 42 && header[2] == 70 && header[3] == 32))
        return data;

    read(fs, data.format);
    read(fs, data.nbVertex);
    read(fs, data.nbIndex);

    data.vData = new renderer::MeshData::DataType[data.nbVertex];
    data.indexData = new uint[data.nbIndex];

    fs.read(reinterpret_cast<char*>(data.vData), sizeof(renderer::MeshData::DataType)*data.nbVertex);
    fs.read(reinterpret_cast<char*>(data.indexData), sizeof(uint)*data.nbIndex);

    return data;
}

void MeshLoader::exportTim(const renderer::MeshData& data, const std::string& file)
{
    std::ofstream fs(file, std::ios_base::binary);
    if(!fs || !data.nbVertex || !data.nbIndex)
        return;

    char header[4] = {43,42,70,32};
    fs.write(header,4);

    write(fs, data.format);
    write(fs, data.nbVertex);
    write(fs, data.nbIndex);

    fs.write(reinterpret_cast<char*>(data.vData), sizeof(renderer::MeshData::DataType)*data.nbVertex);
    fs.write(reinterpret_cast<char*>(data.indexData), sizeof(uint)*data.nbIndex);
}

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
