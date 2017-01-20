#ifndef MESHLOADER_H
#define MESHLOADER_H

#include "Vector.h"
#include "renderer/VertexFormat.h"
#include "renderer/MeshBuffers.h"
#include "renderer/renderer.h"
#include <boost/unordered_map.hpp>

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace resource
{
    class MeshLoader
    {
    public:
        static renderer::MeshData importObj(const std::string&, bool tangent=true);

        static renderer::MeshBuffers* createMeshBuffers(renderer::MeshData&, renderer::VertexBufferPoolType*, renderer::IndexBufferPoolType*);

        static renderer::MeshData importTim(const std::string&);
        static void exportTim(const renderer::MeshData&, const std::string&);

////        static void exportSkeleton(Skeleton*, const vector<SkeletonAnimation*>&, const std::string&);
////        static Skeleton* importSkeleton(const std::string&, vector<SkeletonAnimation*>&);

////        static renderer::MeshBuffers* genGrid(const vec2&, uint, float virtualZ=0);
////        static renderer::MeshBuffers* genGrid(const vec3&, Image*, renderer::VertexFormat);

        static void computeTangent(renderer::MeshData&);

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

        struct hash_uivec3
        {
        public:
            std::size_t operator()(const uivec3& v) const
            {
                boost::hash<uint> hasher;

                return hasher(v[0]) + hasher(v[1]) + hasher(v[2]);
            }
        };

        using VNC_Map = boost::unordered_map<uivec3, size_t, hash_uivec3>;
        //using VNC_Map = boost::container::map<uivec3, size_t>;

        static bool loadObjData(const std::string&, ObjBuffer&);
        static size_t computeObjVertexMap(ObjBuffer&, renderer::MeshData&, VNC_Map&);
        static uivec3 parseObjIndex(const std::string&, bool&, int);

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
