#include "ParticleMesh.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
    using namespace renderer;
namespace interface
{

ParticleMesh::ParticleMesh(uint maxParticles, const std::function<TypeFunc>& fCreate, const std::function<TypeFunc>& fUpdate, int seed)
        : _particles(new Particle[maxParticles]), _size(maxParticles),
          _rand(seed), _funCreate(fCreate), _funUpdate(fUpdate)
{
    _mesh = new renderer::MeshBuffers(vertexBufferPool->alloc(maxParticles), indexBufferPool->alloc(maxParticles), Sphere(vec3(0,0,0), 1));
    _geometry = new interface::Geometry(_mesh);
    _gpuData = new renderer::VNCT_Vertex[maxParticles];

    std::unique_ptr<uint[]> idat(new uint[maxParticles]);
    for(uint i=0 ; i<_size ; ++i)
        _mesh->ib()->flush(idat.get(), 0, maxParticles);
}

ParticleMesh::~ParticleMesh()
{
    delete[] _particles;
    delete [] _gpuData;
    delete _geometry;
}

//void ParticleMesh::benchVolume(float time, float tps)
//{
//    while(time > 0)
//    {
//        update(time);
//        updateBuffer();
//        time -= tps;
//    }

//    for(uint i=0 ; i<_size ; ++i)
//    {
//        _particles[i].lifeTime = 0;
//    }

//    _mesh->vertexBuffer()->computeBoundingVolume();
//}

void ParticleMesh::flushVolume()
{
    _mesh->setVolume(Sphere::computeSphere(reinterpret_cast<real*>(_gpuData), _particlesAlive, sizeof(renderer::MeshData::DataType)/sizeof(float)));
}

void ParticleMesh::update(float time)
{
    _totalTime += time;
    uint nbNewParticles=0;
    if(_flow >= 0)
    {
        float nb = time*_flow;
        nbNewParticles = uint(nb);
        if(_rand.frand() < fmodf(nb, 1.f))
            ++nbNewParticles;
    }
    else nbNewParticles = 0xffff;

    for(uint i=0 ; i<_size ; ++i)
    {
        if(_particles[i].lifeTime > 0)
        {
            _particles[i].bornTime += time;
            _funUpdate(_particles[i], time, _rand);
            _particles[i].lifeTime -= time;
        }
        else if(nbNewParticles > 0)
        {
            _particles[i].id = _currentId++;
            _particles[i].bornTime = 0;
            _funCreate(_particles[i], _totalTime, _rand);
            --nbNewParticles;
        }
    }

}

void ParticleMesh::reset()
{
    for(size_t i=0 ; i<_size ; ++i)
    {
        _particles[i].lifeTime = -1;
    }
    _particlesAlive=0;
    _totalTime=0;
}

void ParticleMesh::updateBuffer() const
{
    VNCT_Vertex* buf = _gpuData;
    uint indexBuf=0;

    for(uint i=0 ; i<_size ; ++i)
    {
        if(_particles[i].lifeTime <= 0)
            continue;

        buf[indexBuf].v = _particles[i].position;
        buf[indexBuf].n = _particles[i].color;
        buf[indexBuf].c = _particles[i].alpha_rotation;
        buf[indexBuf].t = _particles[i].sizeXY_Z;
        indexBuf++;
    }

    _particlesAlive = indexBuf;

}

void ParticleMesh::flush() const
{
    _mesh->vb()->flush(reinterpret_cast<float*>(_gpuData), 0, _particlesAlive);
    _mesh->vb()->setSize(_particlesAlive);
    _mesh->ib()->setSize(_particlesAlive);
}

}
}
#include "MemoryLoggerOff.h"
