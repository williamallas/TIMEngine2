#ifndef INDIRECTLIGHTRENDERER_H
#define INDIRECTLIGHTRENDERER_H

#include "LightContextRenderer.h"
#include "DrawState.h"
#include "resource/Image.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

   class IndirectLightRenderer : boost::noncopyable
   {
   public:
       static Texture* processSkybox(Texture*, Shader*);
       static float* computeBrdf(uint size = 512);

       IndirectLightRenderer(LightContextRenderer&);
       ~IndirectLightRenderer();

       void draw() const;

       Shader* processSkyboxShader() const { return _processCubeMap; }
       Shader* globalIndirectPassShader() const { return _fullScreenPass; }
       void setSkybox(Texture*, Texture*);

   private:
       static const uint NB_MIPMAP = 7;
       static const uint MAX_RESOLUTION = 512;

       LightContextRenderer& _context;

       Shader* _fullScreenPass = nullptr;
       DrawState _stateFullScreenPass;

       Shader* _processCubeMap = nullptr;

       Texture* _skybox = nullptr,
              * _processedSkybox = nullptr,
              * _processedBrdf = nullptr;
   };

   inline void  IndirectLightRenderer::setSkybox(Texture* skybox, Texture* processed)
   {
       _skybox = skybox;
       _processedSkybox = processed;
   }

}
}
#include "MemoryLoggerOff.h"

#endif // INDIRECTLIGHTRENDERER_H
