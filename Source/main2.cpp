#include <SDL_image.h>
#include "TIM_SDL/SDLInputManager.h"
#include "TIM_SDL/SDLTimer.h"
#include "TIM_SDL/SDLTextureLoader.h"
#include "DebugCamera.h"

#include "Rand.h"
#include "interface/ShaderPool.h"
#include "resource/AssetManager.h"

#include "interface/FullPipeline.h"

#include "bullet/BulletEngine.h"

#include "OpenVR\OnHmdRenderer.h"

#include "SpaceBounce.h"

#undef interface
using namespace tim::core;
using namespace tim::renderer;
using namespace tim::resource;
using namespace tim::interface;
using namespace tim;

SDL_Window *pWindow;
SDL_GLContext contexteOpenGL;
void initContextSDL();
void delContextSDL();

const uint WIN_RES_X = 1600;
const uint WIN_RES_Y = 900;

template <int I, int N = 100>
float countTime(float time)
{
	static uint counter = 0;
	static float totalTime = 0;
	static float fps = 0;

	counter++;

	totalTime += time;
	if (counter % N == 0)
	{
		fps = totalTime / N;
		counter = 0;
		totalTime = 0;
		return fps;
	}
	return -fps;
}


int main(int, char**)
{
	uint RES_X = 1600;
	uint RES_Y = 900;

	tim::core::init();
	{
		initContextSDL();
		tim::renderer::init();
		resource::textureLoader = new SDLTextureLoader;

		LOG(openGL.strHardward());
		{
			VR_Device hmdDevice;

            ShaderPool::instance().add("gPass", "shader/gBufferPass.vert", "shader/gBufferPass.frag").value();
            ShaderPool::instance().add("fxaa", "shader/fxaa.vert", "shader/fxaa.frag");

			SDLInputManager input;
			float timeElapsed = 0, totalTime = 0;

			DebugCamera freeFly(&input);

			if (hmdDevice.isInit())
			{
				RES_X = hmdDevice.hmdResolution().x();
				RES_Y = hmdDevice.hmdResolution().y();
				std::cout << "HMD resolution:" << RES_X << "x" << RES_Y << std::endl;
			}

			/* Pipeline entity */
			FullPipeline pipeline;
			FullPipeline::Parameter pipelineParam;
			pipelineParam.useShadow = true;
			pipelineParam.usePointLight = true;
            pipelineParam.shadowCascad = vector<float>({ 4, 15, 100});
            pipelineParam.shadowResolution = 2048;
            pipelineParam.useSSReflexion = true;

			pipeline.createForHmd<OnHmdRenderer>({ uint(RES_X), uint(RES_Y) }, pipelineParam);
			OnHmdRenderer* hmdNode = reinterpret_cast<OnHmdRenderer*>(pipeline.hmdNode);
			hmdNode->setVRDevice(&hmdDevice);
			//hmdNode->setDrawOnScreen(false);

			Pipeline::SceneEntity<SimpleScene> sceneEntity;
			sceneEntity.globalLight.dirLights.push_back({ vec3(1,1,-2), vec4::construct(1), true });

			Pipeline::SceneView sceneViewCulling;
			Pipeline::SceneView sceneViewEye[2];
			sceneViewCulling.camera.ratio = float(RES_X) / RES_Y;
            sceneViewCulling.camera.fov = 140;
			sceneViewCulling.camera.clipDist = { .1f,1000.f };

			for (int i = 0; i < 2; ++i)
			{
				sceneViewEye[i].camera.ratio = float(RES_X) / RES_Y;
				sceneViewEye[i].camera.clipDist = { .1f,1000.f };
				pipeline.hmdEyeRendererNode[i]->setSceneView(sceneViewEye[i]);
				pipeline.hmdEyeRendererNode[i]->setGlobalLight(sceneEntity.globalLight);
				sceneViewEye[i].camera.pos = vec3(0, 0, 0);
                sceneViewEye[i].camera.fov = 120;
			}

			Pipeline::SceneView lightDirView;
			lightDirView.dirLightView.lightDir = vec3(1, 1, -2);
			pipeline.setScene(sceneEntity, sceneViewCulling);
			pipeline.setDirLightView(lightDirView);

			/* Load skybox */
			std::pair<renderer::Texture*, renderer::Texture*> skyboxs;
			TextureLoader::ImageFormat formatTex;

			vector<std::string> imgSkybox(6);
			std::string pre = "skybox/";
			imgSkybox[0] = pre + "x.png";
			imgSkybox[1] = pre + "nx.png";
			imgSkybox[2] = pre + "y.png";
			imgSkybox[3] = pre + "ny.png";
			imgSkybox[4] = pre + "z.png";
			imgSkybox[5] = pre + "nz.png";
			vector<ubyte*> dataSkybox = SDLTextureLoader().loadImageCube(imgSkybox, formatTex);

			renderer::Texture::GenTexParam skyboxParam;
			skyboxParam.format = renderer::Texture::RGBA8;
			skyboxParam.nbLevels = 1;
			skyboxParam.size = uivec3(formatTex.size, 0);
			skyboxs.first = renderer::Texture::genTextureCube(skyboxParam, dataSkybox, 4);
			skyboxs.second = IndirectLightRenderer::processSkybox(skyboxs.first, pipeline.rendererEntity->envLightRenderer().processSkyboxShader());
			for (uint j = 0; j<dataSkybox.size(); ++j)
				delete[] dataSkybox[j];
			
			sceneEntity.globalLight.skybox = { skyboxs.first, skyboxs.second };

            BulletEngine physEngine;

            ThreadPool threadPool(1);
            SpaceBounce demoVR(sceneEntity, physEngine, hmdDevice);

			while (!input.keyState(SDLK_ESCAPE).pressed)
			{
				SDLTimer timer;
                threadPool.wait();	
				input.getEvent();

				demoVR.update(timeElapsed);

				threadPool.schedule([&physEngine, timeElapsed]() { physEngine.dynamicsWorld->stepSimulation(timeElapsed, 5, 1 / 200.f); });

				if (hmdDevice.isInit())
				{
					sceneViewEye[VR_Device::LEFT].camera.useRawMat = true;
					sceneViewEye[VR_Device::RIGHT].camera.useRawMat = true;
					sceneViewEye[VR_Device::LEFT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::LEFT);
                    sceneViewEye[VR_Device::LEFT].camera.raw_view = hmdDevice.camera().eyeView(VR_Device::LEFT) * mat4::Translation(-demoVR.basePosition());
					sceneViewEye[VR_Device::RIGHT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::RIGHT);
                    sceneViewEye[VR_Device::RIGHT].camera.raw_view = hmdDevice.camera().eyeView(VR_Device::RIGHT) * mat4::Translation(-demoVR.basePosition());

					mat4 matInv = hmdDevice.camera().hmdView().inverted();
                    sceneViewCulling.camera.pos = matInv.translation() + demoVR.basePosition();
                    sceneViewEye[VR_Device::RIGHT].camera.pos = sceneViewEye[VR_Device::RIGHT].camera.raw_view.inverted().translation() + demoVR.basePosition();
                    sceneViewEye[VR_Device::LEFT].camera.pos = sceneViewEye[VR_Device::LEFT].camera.raw_view.inverted().translation() + demoVR.basePosition();

					sceneViewCulling.camera.dir = sceneViewCulling.camera.pos + matInv * vec3(0, 0, -5);
					lightDirView.dirLightView.camPos = sceneViewCulling.camera.pos;
				}
				else
				{ 
					freeFly.update(timeElapsed, sceneViewCulling.camera); 

                    mat4 proj = mat4::Projection(120, float(RES_X) / RES_Y, 0.1f, 1000);
					mat4 view = mat4::View(sceneViewCulling.camera.pos, sceneViewCulling.camera.dir, sceneViewCulling.camera.up);
					sceneViewEye[VR_Device::LEFT].camera.useRawMat = true; 
					sceneViewEye[VR_Device::RIGHT].camera.useRawMat = true;
					sceneViewEye[VR_Device::RIGHT].camera.raw_proj = proj;
					sceneViewEye[VR_Device::LEFT].camera.raw_proj = proj;
					sceneViewEye[VR_Device::RIGHT].camera.raw_view = view;
					sceneViewEye[VR_Device::LEFT].camera.raw_view = view;

					lightDirView.dirLightView.camPos = sceneViewCulling.camera.pos;
					sceneViewEye[VR_Device::LEFT].camera.pos = sceneViewCulling.camera.pos;
					sceneViewEye[VR_Device::RIGHT].camera.pos = sceneViewCulling.camera.pos;
					demoVR.setPosDir(sceneViewCulling.camera.pos, sceneViewCulling.camera.dir);
				}


				pipeline.pipeline->prepare();
				pipeline.pipeline->render();

				SDL_GL_SwapWindow(pWindow);
				GL_ASSERT();

				hmdDevice.update();

                if(input.keyState(SDLK_o).firstPress)
                {
                    demoVR.initBall(sceneViewCulling.camera.pos);
                }

                if(input.keyState(SDLK_k).firstPress)
                {
                    demoVR.restartRoom();
                }


				//if (input.keyState(SDLK_1).pressed)
				//	pipeline.onScreen->setBufferOutputNode(pipeline.rendererNode->outputNode(1), 0);
				//if (input.keyState(SDLK_2).pressed)
				//	pipeline.onScreen->setBufferOutputNode(pipeline.rendererNode->outputNode(2), 0);
				//if (input.keyState(SDLK_3).pressed)
				//	pipeline.onScreen->setBufferOutputNode(pipeline.rendererNode->outputNode(3), 0);
				//if (input.keyState(SDLK_4).pressed)
				//	pipeline.onScreen->setBufferOutputNode(pipeline.rendererNode->outputNode(0), 0);
					
				timeElapsed = timer.elapsed()*0.001;
				totalTime += timeElapsed;
				{
					float fps = countTime<0>(timeElapsed);
                   
                    if (fps > 0)
						std::cout << "Fps:" << 1.f / fps << "  Ms:" << 1000 * fps << std::endl;
                   
				}
			}

			/** Close context **/
			threadPool.wait();

			AssetManager<Geometry>::freeInstance();
			AssetManager<interface::Texture>::freeInstance();
			ShaderPool::freeInstance();
			openGL.execAllGLTask();
		}

		delete resource::textureLoader;
		tim::renderer::close();
		delContextSDL();
	}
	tim::core::quit();
	LOG("Quit\n");

	return 0;
}

void initContextSDL()
{
	/* Initialisation simple */
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		LOG("Echec de l'initialisation de la SDL\n"); //out(SDL_GetError()); //out("\n");
		system("pause");
		return;
	}

	//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	pWindow = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WIN_RES_X, WIN_RES_Y,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL /* | SDL_WINDOW_FULLSCREEN*/);
	contexteOpenGL = SDL_GL_CreateContext(pWindow);

	//SDL_ShowCursor(SDL_DISABLE);
    SDL_SetWindowGrab(pWindow, SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

	if (contexteOpenGL == 0)
	{
		LOG(SDL_GetError(), "\n");
		system("pause");
		return;
	}
	openGL.setViewPort({ 0,0 }, { WIN_RES_X, WIN_RES_Y });

	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
}

void delContextSDL()
{
	IMG_Quit();
	SDL_GL_DeleteContext(contexteOpenGL);
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}
