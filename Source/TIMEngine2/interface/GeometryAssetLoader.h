#ifndef GEOMETRYASSETLOADER_H_INCLUDED
#define GEOMETRYASSETLOADER_H_INCLUDED

#include "resource/MeshLoader.h"
#include "resource/AssetLoader.h"
#include "Geometry.h"

namespace tim
{
    using namespace core;
namespace resource
{
    template <>
    class AssetLoader<interface::Geometry>
    {
    public:
        template<bool async>
        Option<interface::Geometry> operator()(std::string file, bool keepData = false)
        {
            if(!async)
            {
                renderer::MeshData* data = new renderer::MeshData;

                if(StringUtils(file).extension() == "obj")
                    *data = MeshLoader::importObj(file);
                else
                    *data = MeshLoader::importTim(file);

                if(data->nbIndex > 0 && data->nbVertex > 0)
                {
                    renderer::VBuffer* vb = renderer::vertexBufferPool->alloc(data->nbVertex);
                    renderer::IBuffer* ib = renderer::indexBufferPool->alloc(data->nbIndex);
                    vb->flush(reinterpret_cast<float*>(data->vData), 0, data->nbVertex);
                    ib->flush(data->indexData, 0, data->nbIndex);

                    renderer::MeshBuffers* mb = nullptr;
                    if(keepData)
                    {
                        mb = new renderer::MeshBuffers(vb, ib, Sphere::computeSphere(reinterpret_cast<real*>(data->vData), data->nbVertex,
                                                       sizeof(renderer::VNC_Vertex)/sizeof(float)), data);
                    }
                    else
                    {
                        mb = new renderer::MeshBuffers(vb, ib, Sphere::computeSphere(reinterpret_cast<real*>(data->vData), data->nbVertex,
                                                       sizeof(renderer::VNC_Vertex)/sizeof(float)));

                        data->clear();
                        delete data;
                    }

                    return Option<interface::Geometry>(interface::Geometry(mb));
                }
                else
                {
                    LOG_EXT("Failed to load Geometry: ", file);
                    data->clear();
                    delete data;
                    return Option<interface::Geometry>();
                }
            }
            else
            {
                renderer::MeshBuffers* emptyBuf = new renderer::MeshBuffers(nullptr, nullptr);
                interface::Geometry geom(emptyBuf);

                auto asyncLoad = [=](){
                    renderer::MeshData* data = new renderer::MeshData;

                    if(StringUtils(file).extension() == "obj")
                        *data = MeshLoader::importObj(file);
                    else
                        *data = MeshLoader::importTim(file);

                    if(data->nbIndex > 0 && data->nbVertex > 0)
                    {
                        interface::Geometry copyGeom = geom;
                        renderer::VBuffer* vb = renderer::vertexBufferPool->alloc(data->nbVertex);
                        renderer::IBuffer* ib = renderer::indexBufferPool->alloc(data->nbIndex);

                        renderer::MeshBuffers* mb = new renderer::MeshBuffers(vb, ib, Sphere::computeSphere(reinterpret_cast<real*>(data->vData), data->nbVertex,
                                                                              sizeof(renderer::VNC_Vertex)/sizeof(float)), keepData ? data : nullptr);
                        emptyBuf->swap(*mb);

                        renderer::openGL.pushGLTask([=](){
                            interface::Geometry copyGeom2 = copyGeom;
                            vb->flush(reinterpret_cast<float*>(data->vData), 0, data->nbVertex);
                            ib->flush(data->indexData, 0, data->nbIndex);

                            if(!keepData)
                            {
                                data->clear();
                                delete data;
                            }
                        });

                        delete mb;
                    }
                    else
                    {
                        LOG_EXT("Failed to load Geometry: ", file);
                        data->clear();
                        delete data;
                    }
                };
                renderer::globalThreadPool.schedule(asyncLoad);

                return Option<interface::Geometry>(geom);
            }
        }
    };
}
}

#endif // GEOMETRYASSETLOADER_H_INCLUDED
