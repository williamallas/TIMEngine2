#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "renderer/DeviceFunctionnality.h"
#include "renderer/GLState.h"
#include "renderer/MeshBuffers.h"
#include "resource/Asset.h"
#include "renderer/MeshBuffers.h"
#include "Sphere.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
    namespace pipeline {
        class DeferredRendererNode;
        class DirLightShadowNode;
    }
}

namespace interface
{
    /** Immutable class */
    class Geometry : public resource::Asset<renderer::MeshBuffers>
    {
        friend class interface::pipeline::DeferredRendererNode;
        friend class interface::pipeline::DirLightShadowNode;

    public:
        using resource::Asset<renderer::MeshBuffers>::Asset;

        Geometry() = default;
        Geometry(renderer::MeshBuffers* ptr) : resource::Asset<renderer::MeshBuffers>(ptr) {}

        size_t nbVertex() const { if(!buffers()) return 0; return _ptr->vb()->size(); }
        size_t nbIndex() const { if(!buffers()) return 0; return _ptr->ib()->size(); }

        Sphere volume() const { if(!buffers()) return Sphere(); return _ptr->volume(); }

        bool isEmpty() const { if(!buffers()) return true; return _ptr->isNull(); }

        const renderer::MeshData* meshData() const { return _ptr->cpuData(); }

    protected:
        renderer::MeshBuffers* buffers() const { return _ptr.get(); }
    };

    /** Mutable version, can be converted into Geometry */
    class MutableGeometry : public Geometry
    {
    public:
        using Geometry::Geometry;
        MutableGeometry() = default;

        MutableGeometry(size_t nbV, size_t nbI=0)
            : Geometry::Geometry(new renderer::MeshBuffers(renderer::vertexBufferPool->alloc(nbV),
                                                           renderer::indexBufferPool->alloc(nbI==0?nbV:nbI)))
        {
            nbI = nbI==0?nbV:nbI;
            boost::shared_array<uint> idat(new uint[nbI]);
            for(uint i=0 ; i<nbI ; ++i) idat[i] = i;

            setIData(idat, 0, nbI);
        }

        using TypeVData = boost::shared_array<const renderer::VertexType>;
        void setVData(TypeVData data, size_t begin, size_t nb)
        {
            if(renderer::getThreadId() == renderer::openGL.getContextId())
                _ptr->vb()->flush(reinterpret_cast<const renderer::VBuffer::Type*>(data.get()), begin, nb);
            else
            {
                auto buffers = this->_ptr;

                renderer::openGL.pushGLTask([=]() {
                    buffers->vb()->flush(reinterpret_cast<const renderer::VBuffer::Type*>(data.get()), begin, nb);
                });
            }

            //_volume = Sphere::computeSphere(reinterpret_cast<const real*>(data.get()), nb, sizeof(renderer::VertexType) / sizeof(float));
        }

        using TypeIData = boost::shared_array<const renderer::IBuffer::Type>;
        void setIData(TypeIData data, size_t begin, size_t nb)
        {
            if(renderer::getThreadId() == renderer::openGL.getContextId())
                _ptr->ib()->flush(data.get(), begin, nb);
            else
            {
                auto buffers = this->_ptr;

                renderer::openGL.pushGLTask([=]() {
                    buffers->ib()->flush(data.get(), begin, nb);
                });
            }
        }
    };
}
}
#include "MemoryLoggerOff.h"

#endif // GEOMETRY_H
