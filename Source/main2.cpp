#include "MainHelper.h"
#include "DebugCamera.h"
#include "MultiSceneManager.h"
#include "resource/AssetManager.h"

#include "interface/FullPipeline.h"
#include "MultipleSceneHelper.h"
#include "bullet/BulletEngine.h"

#include "OpenVR/OnHmdRenderer.h"
#include "OpenVR/HmdSceneView.h"
#include "PortalGame/Controller.h"

#undef interface
using namespace tim::core;
using namespace tim::renderer;
using namespace tim::resource;
using namespace tim::interface;
using namespace tim;

int main(int, char**)
{
    uint RES_X = 1512;
    uint RES_Y = 1680;

	tim::core::init();
	{
		initContextSDL();
		tim::renderer::init();
		resource::textureLoader = new SDLTextureLoader;

        LOG(openGL.strHardward(),"\n");
		{
            ShaderPool::instance().add("gPass", "shader/gBufferPass.vert", "shader/gBufferPass.frag").value();
            ShaderPool::instance().add("gPassAlphaTest", "shader/gBufferPass.vert", "shader/gBufferPass.frag", "", {"ALPHA_TEST"}).value();
            ShaderPool::instance().add("water", "shader/gBufferPass.vert", "shader/gBufferPass.frag", "", {"WATER_SHADER"}).value();
            ShaderPool::instance().add("portalShader", "shader/gBufferPass.vert", "shader/gBufferPass.frag", "", {"PORTAL_SHADER"}).value();
            ShaderPool::instance().add("fxaa", "shader/fxaa.vert", "shader/fxaa.frag").value();
            ShaderPool::instance().add("combineScene", "shader/combineScene.vert", "shader/combineScene.frag").value();
            ShaderPool::instance().add("feedbackStereo", "shader/combineScene.vert", "shader/combineScene.frag", "", {"STEREO_DISPLAY"}).value();
            ShaderPool::instance().add("processSpecularCubeMap", "shader/processCubemap.vert", "shader/processCubemap.frag").value();

            VR_Device hmdDevice;
			if (hmdDevice.isInit())
			{
				RES_X = hmdDevice.hmdResolution().x();
				RES_Y = hmdDevice.hmdResolution().y();
				std::cout << "HMD resolution:" << RES_X << "x" << RES_Y << std::endl;
			}
            else
            {
                std::cout << "HMD not detected !\n";
            }

			/* Pipeline entity */
			FullPipeline pipeline;
			FullPipeline::Parameter pipelineParam;
            pipelineParam.useShadow = false;
            pipelineParam.usePointLight = false;
            pipelineParam.shadowCascad = vector<float>({5, 20});
            pipelineParam.shadowResolution = 1024;
            pipelineParam.useSSReflexion = false;
            pipelineParam.useFxaa = false;

            OnHmdRenderer* hmdNode = new OnHmdRenderer;
            hmdNode->setDrawOnScreen(2);
            float ratio = float(RES_X)/RES_Y;
            hmdNode->setScreenResolution({WIN_RES_X, WIN_RES_Y});
            hmdNode->setShader(ShaderPool::instance().get("feedbackStereo"));

            pipeline.createStereoExtensible(*hmdNode, {RES_X,RES_Y}, pipelineParam);
            hmdNode->setVRDevice(&hmdDevice);

            HmdSceneView hmdCamera(110, ratio, 100);

            /* physic and setup */
            BulletEngine physEngine;
            ThreadPool threadPool(4);

            SDLInputManager input;
            float timeElapsed = 0, totalTime = 0;

            DebugCamera freeFly(&input);

            // Setup and load scenes + multiple portals
            pipeline.setStereoView(hmdCamera.cullingView(), hmdCamera.eyeView(0), hmdCamera.eyeView(1), 0);

            MultipleSceneHelper portalManager(pipelineParam, pipeline);

            portalManager.setResolution({RES_X,RES_Y});
            portalManager.setView(hmdCamera.cullingView());
            portalManager.setStereoView(hmdCamera.eyeView(0), hmdCamera.eyeView(1));

            MultiSceneManager allSceneManager("configScene.txt", portalManager);
            if(portalManager.curScene() == nullptr)
            {
                std::cout << "Unable to setup the first scene.\n";
                return -1;
            }
            allSceneManager.instancePhysic(physEngine);

            interface::XmlMeshAssetLoader gameAssets;
            gameAssets.load("gameAssets.xml");

            Controller controllerManager(gameAssets.getMesh("controller", interface::Texture::genParam(true,true,true, 4)), physEngine);
            controllerManager.setControllerOffset(mat4::RotationX(toRad(-86.1672))*mat4::Translation({0, 0.121448f*0.6f, -0.020856f*0.6f}));
            controllerManager.buildForScene(*portalManager.curScene(), allSceneManager.getSceneIndex(portalManager.curScene()));

            hmdDevice.sync();

			while (!input.keyState(SDLK_ESCAPE).pressed)
			{
				SDLTimer timer;

                input.getEvent();
                threadPool.wait();	

                /*********************/
                /******* Flush *******/
                /*********************/

                //threadPool.schedule([&physEngine, timeElapsed]() { physEngine.dynamicsWorld->stepSimulation(timeElapsed, 5, 1 / 200.f); });

				if (hmdDevice.isInit())
				{
                    SDLTimer rup;
                    hmdDevice.update(input.keyState(SDLK_y).pressed);
                    std::cout<< "hmd update : " << rup.elapsed() << std::endl;

                    hmdCamera.update(hmdDevice);

                    if(hmdDevice.isControllerConnected(0) && hmdDevice.isControllerConnected(1))
                        controllerManager.update(hmdCamera.offset(), hmdDevice.controllerPose(VR_Device::LEFT), hmdDevice.controllerPose(VR_Device::RIGHT));
				}
				else
				{ 
                    freeFly.update(timeElapsed, hmdCamera.cullingView().camera);
                    hmdCamera.update(hmdCamera.cullingView().camera);
				}

                interface::Scene* switchScene = nullptr;
                mat4 o;
                if(portalManager.update(switchScene, &o))
                {
                    hmdCamera.addOffset(o);
                    if(hmdDevice.isInit())
                    {
                        controllerManager.buildForScene(*switchScene, allSceneManager.getSceneIndex(portalManager.curScene()));

                        if(hmdDevice.isControllerConnected(0) && hmdDevice.isControllerConnected(1))
                            controllerManager.update(hmdCamera.offset(), hmdDevice.controllerPose(VR_Device::LEFT), hmdDevice.controllerPose(VR_Device::RIGHT));
                    }
                }

                /********************/
                /******* Draw *******/
                /********************/

                pipeline.pipeline()->prepare();
                pipeline.pipeline()->render();
                swapBuffer();

                GL_ASSERT();

                if(input.keyState(SDLK_p).firstPress)
                    std::cout << "Cam pos:" << hmdCamera.cullingView().camera.pos << std::endl;

				timeElapsed = timer.elapsed()*0.001;
				totalTime += timeElapsed;
                pipeline.pipeline()->meshRenderer().frameState().setTime(totalTime, timeElapsed);

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
