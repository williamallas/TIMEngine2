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
       struct Light
       {
           vec3 direction;
           vec3 color;
           renderer::Texture* depthMap = nullptr;
           vector<mat4> matrix;
       };

       static Texture* processSkybox(Texture*, Shader*);
       static float* computeBrdf(uint size = 512);

       static std::pair<Texture*, Texture*> loadAndProcessSkybox(const vector<std::string>&, Shader*);

       IndirectLightRenderer(LightContextRenderer&);
       ~IndirectLightRenderer();

       void draw(const vector<Light>&) const;

       Shader* globalIndirectPassShader() const { return _fullScreenPass; }
       void setSkybox(Texture*, Texture*);
       void setReflexionBuffer(Texture*);

       void setEnableGI(bool b) { _enableGI = b; }
       void setGlobalAmbient(const vec4& col) { _globalAmbient = col; }
       void setEnableSSReflexion(bool b) { _enableSSReflexion = b; }

       bool isLocalReflexionEnabled() const { return _enableSSReflexion; }

   private:
       static const uint NB_MIPMAP = 7;
       static const uint MAX_RESOLUTION = 512;

       LightContextRenderer& _context;

       Shader* _fullScreenPass = nullptr;
       int _uniformEnableGI = -1;
       int _uniformGlobalAmbient = -1;
       int _uniformSSReflexion = -1;

       int _uniformNbLight, _uniformLightDir, _uniformLightColor, _uniformLightMatrix;

       bool _enableGI = false;
       bool _enableSSReflexion = false;
       vec4 _globalAmbient = vec4::construct(0.3); // if enableGI = false

       DrawState _stateFullScreenPass;

       Texture* _skybox = nullptr,
              * _processedSkybox = nullptr,
              * _processedBrdf = nullptr,
              * _inReflexionBuffer = nullptr;
   };

   inline void  IndirectLightRenderer::setSkybox(Texture* skybox, Texture* processed)
   {
       _skybox = skybox;
       _processedSkybox = processed;
   }

   inline void  IndirectLightRenderer::setReflexionBuffer(Texture* tex)
   {
       _inReflexionBuffer = tex;
   }

}
}
#include "MemoryLoggerOff.h"

#endif // INDIRECTLIGHTRENDERER_H
