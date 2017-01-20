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

    VNC_Map mapIndex;

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
        meshData.vData[it.second] = { it.first[0]==0 ? vec3(0,0,0) : buf.vbuffer[it.first[0]-1],
                                      it.first[2]==0 ? vec3(0,0,0) : buf.nbuffer[it.first[2]-1],
                                      it.first[1]==0 ? vec2(0,0)   : buf.tbuffer[it.first[1]-1], vec3() };
	}

    if(meshData.format == renderer::VertexFormat::VNCT)
    {
        computeTangent(meshData);
    }

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

#include "MemoryLoggerOff.h"
class IterateLine
{
public:
    IterateLine() = delete;
    IterateLine(const IterateLine&) = delete;

    IterateLine(std::unique_ptr<char[]>&& buf, size_t size) : buffer(std::move(buf)), sizeBuf(size) {}

    bool nextLine(std::string& str)
    {
        str.clear();
        while(index < sizeBuf)
        {
            if(finish)
                return false;

            if(buffer[index] == '\n')
            {
                ++index; return true;
            }
            else if(buffer[index] == '\0')
            {
                finish = true;
                return str.size() > 0;
            }

            str += buffer[index++];
        }

        finish = true;
        return str.size() > 0;
    }

    void reset()
    {
        index = 0;
        finish = false;
    }

    static bool startWith(const std::string& str, const std::string& prefix)
    {
        if(str.size() < prefix.size())
            return false;
        else return std::equal(prefix.begin(), prefix.end(), str.begin());
    }

    std::unique_ptr<char[]> buffer;
    size_t sizeBuf;
    size_t index=0;
    bool finish = false;
};
#include "MemoryLoggerOn.h"

bool MeshLoader::loadObjData(const std::string& file, ObjBuffer& buffer)
{
    std::ifstream f_tmp(file, std::ios_base::binary);
    f_tmp.seekg (0, f_tmp.end);
    size_t sizeFile = f_tmp.tellg();
    f_tmp.seekg (0, f_tmp.beg);

    auto buf = std::unique_ptr<char[]>(new char[sizeFile]);
    f_tmp.read(&buf[0], sizeFile);
    f_tmp.close();

    IterateLine iterline(std::move(buf), sizeFile);

    buffer.nbVertex=0;
    buffer.nbNormal=0;
    buffer.nbTexCoord=0;
    buffer.nbIndex=0;

    {
        std::string s;

        while(iterline.nextLine(s))
        {
            if(IterateLine::startWith(s,"v "))
                ++buffer.nbVertex;
            else if(IterateLine::startWith(s,"vn "))
                ++buffer.nbNormal;
            else if(IterateLine::startWith(s,"vt "))
                ++buffer.nbTexCoord;
            else if(IterateLine::startWith(s,"f "))
                buffer.nbIndex+=3;
        }
    }

    if(!buffer.nbVertex || !buffer.nbIndex)
        return false;

    buffer.vbuffer = new vec3[buffer.nbVertex];
    buffer.ibuffer = new uivec3[buffer.nbIndex];

    if(buffer.nbNormal)
        buffer.nbuffer = new vec3[buffer.nbNormal];

    if(buffer.nbTexCoord)
        buffer.tbuffer = new vec2[buffer.nbTexCoord];


    size_t vindex=0, nindex=0, tindex=0, iindex=0;
    iterline.reset();
    std::string s;

    while(iterline.nextLine(s))
    {
        if(IterateLine::startWith(s, "v "))
        {
            if(sscanf(s.c_str()+2, "%f %f %f", &(buffer.vbuffer[vindex][0]), &(buffer.vbuffer[vindex][1]), &(buffer.vbuffer[vindex][2])) == 3)
                ++vindex;
            else
            {
                buffer.free();
                return false;
            }
        }

        else if(IterateLine::startWith(s, "vn "))
        {
            if(sscanf(s.c_str()+3, "%f %f %f", &(buffer.nbuffer[nindex][0]), &(buffer.nbuffer[nindex][1]), &(buffer.nbuffer[nindex][2])) == 3)
                ++nindex;
            else
            {
                buffer.free();
                return false;
            }
        }

        else if(IterateLine::startWith(s, "vt "))
        {
            if(sscanf(s.c_str()+3, "%f %f", &(buffer.tbuffer[tindex][0]), &(buffer.tbuffer[tindex][1])) == 2)
                ++tindex;
            else
            {
                buffer.free();
                return false;
            }
        }
        else if(IterateLine::startWith(s, "f "))
        {
            char v[3][64];
            if(sscanf(s.c_str()+2, "%s %s %s", v[0],v[1],v[2]) != 3)
            {
                buffer.free();
                return false;
            }

            for(size_t i=0 ; i<3 ; ++i)
            {
//                bool b=true;
//                buffer.ibuffer[iindex] = parseObjIndex(str, b, nbSlash);
//                if(!b || buffer.ibuffer[iindex].x()>buffer.nbVertex || (buffer.ibuffer[iindex].x()==0 && buffer.nbVertex)
//                      || buffer.ibuffer[iindex].z()>buffer.nbNormal || (buffer.ibuffer[iindex].z()==0 && buffer.nbNormal)
//                      || buffer.ibuffer[iindex].y()>buffer.nbTexCoord || (buffer.ibuffer[iindex].y()==0 && buffer.nbTexCoord))
//                {
//                    buffer.free();
//                    return false;
//                }

                buffer.ibuffer[iindex] = {0,0,0};
                if(buffer.nbNormal == 0 && buffer.nbTexCoord == 0)
                {
                    sscanf(v[i], "%u", &buffer.ibuffer[iindex][0]);
                }
                else
                {
                    switch(sscanf(v[i], "%u/%u/%u", &(buffer.ibuffer[iindex][0]), &(buffer.ibuffer[iindex][1]), &(buffer.ibuffer[iindex][2])))
                    {
                    case 0:
                    default:
                        buffer.free();
                        return false;

                    case 1:
                        buffer.ibuffer[iindex] = {0,0,0};
                        sscanf(v[i], "%u//%u", &(buffer.ibuffer[iindex][0]), &(buffer.ibuffer[iindex][2]));

                    case 2:
                    case 3:
                        break;
                    }
                }

                ++iindex;
            }
        }
    }

//    for(int i=0 ; i<buffer.nbIndex ; ++i)
//    {
//        if(buffer.ibuffer[i].z() >  buffer.nbVertex)
//        {
//            std::cout << "Bug at " << i << ":" << buffer.ibuffer[i]  << " ; nbV:" << buffer.nbVertex <<std::endl;
//        }
//    }

    return true;
}

size_t MeshLoader::computeObjVertexMap(ObjBuffer& buf, renderer::MeshData& meshData,
                                       VNC_Map& mapIndex)
{
    size_t curIndex=0;
    meshData.nbIndex = buf.nbIndex;
    meshData.indexData = new uint[buf.nbIndex];

    //renderer::VNC_Vertex vnc;
    for(size_t i=0 ; i<buf.nbIndex ; ++i)
    {
        //vnc.v = buf.vbuffer[buf.ibuffer[i].x()-1];
        //vnc.n=vec3(); vnc.c=vec2();

//        if(buf.nbNormal && buf.ibuffer[i].z() > 0)
//            vnc.n = buf.nbuffer[buf.ibuffer[i].z()-1];
//        if(buf.nbTexCoord && buf.ibuffer[i].y() > 0)
//            vnc.c = buf.tbuffer[buf.ibuffer[i].y()-1];

        auto it = mapIndex.find(buf.ibuffer[i]);
        if(it == mapIndex.end())
        {
            mapIndex[buf.ibuffer[i]] = curIndex;
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
