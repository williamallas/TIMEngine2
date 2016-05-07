#include <SDL_image.h>
#include "TIM_SDL/SDLInputManager.h"
#include "TIM_SDL/SDLTimer.h"
#include "TIM_SDL/SDLTextureLoader.h"
#include "DebugCamera.h"

#include "Rand.h"
#include "interface/ShaderPool.h"
#include "resource/MeshLoader.h"
#include "resource/AssetManager.h"

#include "interface/FullPipeline.h"

#include "RTSEngine/Graphic/TerrainRenderer.h"
#include "RTSEngine/Graphic/RTSCamera.h"

#include "OpenVR\OnHmdRenderer.h"

#undef interface;
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

template <int I, int N = 50>
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

			Geometry geometry[3];
			geometry[0] = AssetManager<Geometry>::instance().load<false>("m2.obj").value();
			geometry[1] = AssetManager<Geometry>::instance().load<false>("sphere.tim").value();
			geometry[2] = AssetManager<Geometry>::instance().load<false>("cube_uv.obj").value();

			renderer::Texture::GenTexParam texParam;
			texParam.format = renderer::Texture::Format::RGBA8;
			texParam.nbLevels = -1;
			texParam.repeat = true; texParam.linear = true; texParam.trilinear = true;

			interface::Texture solTexture = AssetManager<interface::Texture>::instance().load<false>("grass_pure.png", texParam).value();
			interface::Texture solNrmTexture = AssetManager<interface::Texture>::instance().load<false>("grass_pure_NRM.png", texParam).value();

			interface::Texture m2Tex = AssetManager<interface::Texture>::instance().load<false>("M2AO.png", texParam).value();

			Shader* gPass = ShaderPool::instance().add("gPass", "shader/gBufferPass.vert", "shader/gBufferPass.frag").value();
			ShaderPool::instance().add("terrain", "shader/terrain.vert", "shader/terrain.frag");

			SDLInputManager input;
			float timeElapsed = 0, totalTime = 0;

			DebugCamera freeFly(&input);
			RTSCamera rtsCamera;
			rtsCamera.setResolution({ int(RES_X), int(RES_Y) });
			bool chooseCamera = false;

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
			pipelineParam.shadowCascad = vector<float>({ 5, 40, 300});
			pipelineParam.shadowResolution = 2048;

			pipeline.createForHmd<OnHmdRenderer>({ uint(RES_X), uint(RES_Y) }, pipelineParam);
			OnHmdRenderer* hmdNode = reinterpret_cast<OnHmdRenderer*>(pipeline.hmdNode);
			hmdNode->setVRDevice(&hmdDevice);
			//pipeline.rendererEntity->envLightRenderer().setEnableGI(false);
			//pipeline.rendererEntity->envLightRenderer().setGlobalAmbient(vec4::construct(0.3));

			Pipeline::SceneEntity<SimpleScene> sceneEntity;
			sceneEntity.globalLight.dirLights.push_back({ vec3(1,1,-2), vec4::construct(1), true });

			Pipeline::SceneView sceneViewCulling;
			Pipeline::SceneView sceneViewEye[2];
			sceneViewCulling.camera.ratio = float(RES_X) / RES_Y;
			sceneViewCulling.camera.fov = 110;
			sceneViewCulling.camera.clipDist = { .1f,1000.f };

			for (int i = 0; i < 2; ++i)
			{
				sceneViewEye[i].camera.ratio = float(RES_X) / RES_Y;
				sceneViewEye[i].camera.clipDist = { .1f,1000.f };
				pipeline.hmdEyeRendererNode[i]->setSceneView(sceneViewEye[i]);
				pipeline.hmdEyeRendererNode[i]->setGlobalLight(sceneEntity.globalLight);
				sceneViewEye[i].camera.pos = vec3(0, 0, 0);
			}

			Pipeline::SceneView lightDirView;
			lightDirView.dirLightView.lightDir = vec3(1, 1, -2);
			pipeline.setScene(sceneEntity, sceneViewCulling);
			pipeline.setDirLightView(lightDirView);

			/* Load skybox */
			vector<renderer::Texture*> skyboxes(4);
			vector<renderer::Texture*> processedSky(skyboxes.size());
			{
				TextureLoader::ImageFormat formatTex;

				for (size_t i = 0; i<skyboxes.size(); ++i)
				{
					vector<std::string> imgSkybox(6);
					std::string pre = "skybox/simple" + StringUtils(i + 1).str() + "/";
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
					skyboxes[i] = renderer::Texture::genTextureCube(skyboxParam, dataSkybox, 4);
					processedSky[i] = IndirectLightRenderer::processSkybox(skyboxes[i],
						pipeline.rendererEntity->envLightRenderer().processSkyboxShader());
					for (uint j = 0; j<dataSkybox.size(); ++j)
						delete[] dataSkybox[j];
				}
			}

			sceneEntity.globalLight.skybox = { skyboxes[3], processedSky[3] };

			const float TERRAIN_SIZE = 128;
			const float TERRAIN_H = 50;
			//TerrainRenderer terrain(TERRAIN_SIZE, TERRAIN_H, sceneEntity.scene);
			//ImageAlgorithm<float> physicHM = terrain.patch()->heightData().map([](vec3 v) -> float { return v.z(); });
			//btHeightfieldTerrainShape* heightFieldShape = tim::createHeightFieldShape(vec3(TERRAIN_SIZE, TERRAIN_SIZE, TERRAIN_H), physicHM);
			//BulletObject objHM(mat4::Translation({ 0,0,TERRAIN_H*0.5 }), heightFieldShape);
			//physEngine.addObject(&objHM);
			//objHM.body()->setFriction(10);

			Mesh::Element elem;
			elem.setGeometry(geometry[0]);
			elem.setTexture(m2Tex, 0);
			elem.setRougness(0.7);
			elem.setMetallic(0);
			elem.setColor(vec4::construct(1));
			elem.drawState().setShader(gPass);
			MeshInstance& m2Mesh = sceneEntity.scene.add<MeshInstance>(Mesh(elem), mat4::Scale(vec3(.7f,.7f,.7f)));
			bool isRunning = false;

			elem.setTexture(interface::Texture(), 0);
			elem.setColor(vec4::construct(0.7));
			elem.setRougness(0.5);
			elem.setMetallic(0);
			elem.setGeometry(geometry[2]);

			for(int i=-4 ; i<100 ; ++i)for (int j = 1; j<6; ++j)for (int k = -4; k<6; ++k)
				sceneEntity.scene.add<MeshInstance>(Mesh(elem), mat4::Translation(vec3(i,j,k)*3));

			vec3 pos;
			int indexSkybox = 0;
			while (!input.keyState(SDLK_ESCAPE).pressed)
			{
				SDLTimer timer;
				input.getEvent();

				if (hmdDevice.isInit())
				{
					float z = 0;// terrain.patch()->height(vec2(0, 0));
					sceneViewEye[VR_Device::LEFT].camera.useRawMat = true;
					sceneViewEye[VR_Device::RIGHT].camera.useRawMat = true;
					sceneViewEye[VR_Device::LEFT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::LEFT);
					sceneViewEye[VR_Device::LEFT].camera.raw_view = hmdDevice.camera().eyeView(VR_Device::LEFT) * mat4::Translation(-vec3(pos));
					sceneViewEye[VR_Device::RIGHT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::RIGHT);
					sceneViewEye[VR_Device::RIGHT].camera.raw_view = hmdDevice.camera().eyeView(VR_Device::RIGHT) * mat4::Translation(-vec3(pos));

					sceneViewCulling.camera.pos = hmdDevice.camera().hmdView().inverted() * vec3(0, 0, 0);
					sceneViewEye[VR_Device::RIGHT].camera.pos = sceneViewEye[VR_Device::RIGHT].camera.raw_view.inverted() * vec3(0, 0, 0);
					sceneViewEye[VR_Device::LEFT].camera.pos = sceneViewEye[VR_Device::LEFT].camera.raw_view.inverted() * vec3(0, 0, 0);
					
					//sceneViewEye[VR_Device::RIGHT].camera.pos = camPos;
					//sceneViewEye[VR_Device::RIGHT].camera.dir = camDir;

					//invView = sceneViewEye[VR_Device::RIGHT].camera.raw_view.inverted();
					//camPos = invView*vec3(0, 0, 0);
					//camDir = invView*vec3(0, 0, -1);
					//sceneViewEye[VR_Device::LEFT].camera.pos = camPos;
					//sceneViewEye[VR_Device::LEFT].camera.dir = camDir;

					
					//sceneViewCulling.camera.dir = camDir;
				}
				else
				{
					freeFly.update(timeElapsed, sceneViewCulling.camera);
					//sceneViewCulling.camera.pos.z() = terrain.patch()->height(sceneViewCulling.camera.pos.to<2>()) + 2;

					mat4 proj = mat4::Projection(90, float(RES_X) / RES_Y, 0.1f, 1000);
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

				}

				if (input.keyState(SDLK_SPACE).firstPress)
				{
					isRunning = true;
				}
				if (isRunning)
				{	
					mat4 m = mat4::Scale(vec3::construct(0.7f));
					m.setTranslation(pos);
					m2Mesh.setMatrix(m);
					if (pos.x() > 280)
						pos.x() = 0;

					pos.x() += 0.3 * timeElapsed;
				}

			

				//if (chooseCamera)
				//{
				//	freeFly.update(timeElapsed, sceneView1.camera);

				//	sceneView1.camera.pos.z() = terrain.patch()->height(sceneView1.camera.pos.to<2>()) + 2;
				//}
				//else
				//{
				//	rtsCamera.setMouseParameter(input.mousePos(), input.mouseWheel().y());
				//	rtsCamera.update(timeElapsed, sceneView2.camera);
				//}

				//if (input.keyState(SDLK_c).firstPress)
				//	chooseCamera = !chooseCamera;




				pipeline.pipeline->prepare();
				pipeline.pipeline->render();

				SDL_GL_SwapWindow(pWindow);
				GL_ASSERT();

				openGL.clearColor({ 0,0,0,0 });

				hmdDevice.update();

				if (input.keyState(SDLK_RIGHT).firstPress)
				{
					indexSkybox = (indexSkybox + 1) % skyboxes.size();
					sceneEntity.globalLight.skybox = { skyboxes[indexSkybox], processedSky[indexSkybox] };
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
					if (fps>0)
					{
						std::cout << "Fps:" << 1.f / fps << "  Ms:" << 1000 * fps << std::endl;
					}
				}
			}

			/** Close context **/
			for (uint i = 0; i<processedSky.size(); ++i)
			{
				delete processedSky[i];
				delete skyboxes[i];
			}

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
	//SDL_SetWindowGrab(pWindow, SDL_TRUE);
	//SDL_SetRelativeMouseMode(SDL_TRUE);

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
