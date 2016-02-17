#ifndef MESHLOADER_H
#define MESHLOADER_H

#include "Vector.h"
#include <fstream>
#include "renderer/VertexFormat.h"
#include "renderer/MeshBuffers.h"
#include "renderer/renderer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace resource
{

    class MeshLoader
    {
    public:
        struct LoadedMeshData
        {
            std::string name;

            uint nbIndex = 0;
            uint* indexData = nullptr;

            renderer::VertexFormat format = renderer::VertexFormat::VNCT;
            uint nbVertex = 0;
            renderer::VNCT_Vertex* vData = nullptr;

            void clear()
            {
                nbIndex=0;
                nbVertex=0;
                delete[] indexData;
                delete[] vData;
                indexData=nullptr;
                vData=nullptr;
            }
        };

        static LoadedMeshData importObj(const std::string&, bool tangent=true);

        static renderer::MeshBuffers* createMeshBuffers(LoadedMeshData&, renderer::VertexBufferPoolType*, renderer::IndexBufferPoolType*);

//        static MeshBuffers* importMeshTim(const std::string&);
//        static void exportTim(MeshBuffers*, const std::string&);

////        static void exportSkeleton(Skeleton*, const vector<SkeletonAnimation*>&, const std::string&);
////        static Skeleton* importSkeleton(const std::string&, vector<SkeletonAnimation*>&);

////        static renderer::MeshBuffers* genGrid(const vec2&, uint, float virtualZ=0);
////        static renderer::MeshBuffers* genGrid(const vec3&, Image*, renderer::VertexFormat);

        static void computeTangent(LoadedMeshData&);

    private:
        MeshLoader() = default;
        ~MeshLoader() = default;

        struct ObjBuffer
        {
            vec3 *vbuffer=nullptr, *nbuffer=nullptr;
            vec2 *tbuffer=nullptr;
            uivec3 *ibuffer=nullptr;
            size_t nbVertex=0, nbNormal=0, nbTexCoord=0, nbIndex=0;
            renderer::VertexFormat format=renderer::VertexFormat::VN;

            void free()
            {
                delete[] vbuffer; vbuffer=nullptr;
                delete[] nbuffer; nbuffer=nullptr;
                delete[] tbuffer; tbuffer=nullptr;
                delete[] ibuffer; ibuffer=nullptr;
                nbVertex=0; nbNormal=0; nbTexCoord=0; nbIndex=0;
            }
        };

        static bool loadObjData(const std::string&, ObjBuffer&);
        static size_t computeObjVertexMap(ObjBuffer&, LoadedMeshData&, boost::container::map<renderer::VNC_Vertex, size_t>&);
        static uivec3 parseObjIndex(const std::string&, bool&, int);

////        static SkeletonAnimation* readSkeletonAnimation(std::istream&);

//        static void writeTim(std::ostream&, renderer::MeshBuffers*);
//        static renderer::MeshBuffers* readMeshTim(std::istream&);

        template<class T> static void write(std::ostream&, const T&);
        template<class T> static void read(std::istream&, T&);

        static void write(std::ostream&, const std::string&);
        static void read(std::istream&, std::string&);
    };

    template<class T>
    void MeshLoader::write(std::ostream& stream, const T& data)
    {
        stream.write(reinterpret_cast<const char*>(&data), sizeof(data));
    }

    template<class T>
    void MeshLoader::read(std::istream& stream, T& data)
    {
        stream.read(reinterpret_cast<char*>(&data), sizeof(data));
    }

}
}
#include "MemoryLoggerOff.h"

#endif // MESHLOADER_H
