#include <SDL.h>
#include "SDLTimer.h"
#include "SDLInputManager.h"
#include "SDLTextureLoader.h"
#include "SDLTimer.h"

#include "interface/EnginePackage.h"
#include "DebugCamera.h"
#include "raytracer/RaytracerRenderer.h"

using namespace tim::core;
using namespace tim::resource;
using namespace tim::renderer;
using namespace tim::interface;
using namespace tim;

extern SDL_Window *pWindow;
extern SDL_GLContext contexteOpenGL;
void initContextSDL();
void delContextSDL();

#define RES_X 1600.f
#define RES_Y 900.f

#define RAY_RES_X 800.f
#define RAY_RES_Y 456.f

int main(int, char**)
{

    tim::core::init();
{
    initContextSDL();
    tim::renderer::init();

    EnginePackage engine(new SDLTextureLoader);
    engine.resource_manager.loadXmlDataInformation("shader\\metashader.xml");
    Material* drawFinal = engine.material_drawTexture->clone();
    drawFinal->setNbTexture(1);

    /** Point of view **/
    Camera camera1;
    camera1.pos = vec3(-2,-2,2);
    camera1.up = vec3(0,0,1);
    camera1.fov = 70.f;
    camera1.ratio = RAY_RES_X/RAY_RES_Y;
    camera1.clipDist = vec2(1,1024);

    SDLInputManager input;
    DebugCamera freeCamera(&input);

    float timeElapsed=0, totalTime=0;

    ThreadPool poolTest(3);

    RaytracerRenderer raytracer({(uint)RES_X,(uint)RES_Y}, 0);

    while(!input.keyState(SDLK_ESCAPE).pressed)
    {
        SDLTimer timer;
        input.getEvent();

        freeCamera.update(timeElapsed, camera1);

        raytracer.setupCamera(camera1, {0,0,0});
        raytracer.draw();

        drawFinal->setTexture(raytracer.buffer(), 0);
        MaterialRenderer().draw(drawFinal, mat4::IDENTITY());
        SDL_GL_SwapWindow(pWindow);

        GL_ASSERT();

        timeElapsed = timer.elapsed()*0.001;
        totalTime += timeElapsed;
    }

    /** Close context **/

    tim::resource::ResourceSceneManager::freeInstance();
    tim::renderer::close();
    delContextSDL();
}
    tim::core::quit();
    err("Quit\n");

    return 0;
}

