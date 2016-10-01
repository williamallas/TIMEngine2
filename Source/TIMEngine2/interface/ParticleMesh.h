#ifndef PARTICLEMESH_H
#define PARTICLEMESH_H

#include "renderer/MeshBuffers.h"
#include "Geometry.h"
#include "Particle.h"

#include <functional>

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{

    class ParticleMesh
    {
    public:

        using TypeFunc = void(Particle&, float, Rand&);

        ParticleMesh(uint, const std::function<TypeFunc>&, const std::function<TypeFunc>&, int seed=0);
        virtual ~ParticleMesh();

//        void benchVolume(float time = 5, float tps = 0.1);
        void flushVolume();

        interface::Geometry geometry() const;

        void setFlow(float);
        float flow() const;

        void update(float);
        void reset();

        void updateBuffer() const;
        void flush() const;

    private:
        Particle* _particles;
        renderer::VNCT_Vertex* _gpuData;
        uint _size;

        mutable uint _particlesAlive = 0;
        renderer::MeshBuffers* _mesh;
        interface::Geometry* _geometry;

        uint _currentId = 1;
        float _totalTime = 0;
        Rand _rand;

        float _flow = -1; // particles / sec

        std::function<TypeFunc> _funCreate;
        std::function<TypeFunc> _funUpdate;

        #include "MemoryLoggerOff.h"
        ParticleMesh& operator=(const ParticleMesh&) = delete;
        ParticleMesh(const ParticleMesh&) = delete;
        #include "MemoryLoggerOn.h"
    };

    inline void ParticleMesh::setFlow(float f) { _flow=f; }
    inline float ParticleMesh::flow() const { return _flow; }

    inline interface::Geometry ParticleMesh::geometry() const { return *_geometry; }

}
}
#include "MemoryLoggerOff.h"

#endif // PARTICLEMESH_H
