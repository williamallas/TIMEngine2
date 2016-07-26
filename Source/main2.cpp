#include "MainHelper.h"
#include "DebugCamera.h"
#include "MultiSceneManager.h"
#include "resource/AssetManager.h"

#include "interface/FullPipeline.h"
#include "MultipleSceneHelper.h"
#include "bullet/BulletEngine.h"

#include "OpenVR/OnHmdRenderer.h"
#include "OpenVR/HmdSceneView.h"

#undef interface
using namespace tim::core;
using namespace tim::renderer;
using namespace tim::resource;
using namespace tim::interface;
using namespace tim;

int main(int, char**)
{
    uint RES_X = 1080;
    uint RES_Y = 1200;

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
            pipelineParam.useShadow = true;
            pipelineParam.usePointLight = false;
            pipelineParam.shadowCascad = vector<float>({5, 20});
            pipelineParam.shadowResolution = 1024;
            pipelineParam.useSSReflexion = true;
            pipelineParam.useFxaa = true;

            OnHmdRenderer* hmdNode = new OnHmdRenderer;
            hmdNode->setDrawOnScreen(2);
            hmdNode->setScreenResolution({WIN_RES_X, WIN_RES_Y});
            hmdNode->setShader(ShaderPool::instance().get("feedbackStereo"));

            pipeline.createStereoExtensible(*hmdNode, {RES_X,RES_Y}, pipelineParam);
            hmdNode->setVRDevice(&hmdDevice);

            HmdSceneView hmdCamera(110, float(RES_X)/RES_Y);

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

//            MeshInstance* portal1 = nullptr;
//            for(auto o : objInScene1)
//            {
//                if(o.name == "portal")
//                {
//                    portal1 = o.meshInstance;
//                }
//            }

//            MeshInstance* portal2 = nullptr;
//            for(auto o : objInScene2)
//            {
//                if(o.name == "portal")
//                {
//                    portal2 = o.meshInstance;
//                }
//            }

//            vec3 tr = portal2->matrix().translation() - portal1->matrix().translation();
//            portalManager.addEdge(&scene1, &scene2, portal1, AssetManager<Geometry>::instance().load<false>("meshBank/portal1.obj", true).value(), tr);
//            portalManager.addEdge(&scene2, &scene1, portal2, AssetManager<Geometry>::instance().load<false>("meshBank/portal1.obj", true).value(), -tr);

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
                    hmdDevice.update();
                    hmdCamera.update(hmdDevice, vec3());

                    // flush pipeline component
				}
				else
				{ 
                    freeFly.update(timeElapsed, hmdCamera.cullingView().camera);
                    hmdCamera.update(hmdCamera.cullingView().camera);
				}

                interface::Scene* switchScene = nullptr;
                portalManager.update(switchScene);

                /********************/
                /******* Draw *******/
                /********************/

                pipeline.pipeline()->prepare();
                pipeline.pipeline()->render();

                swapBuffer();
				GL_ASSERT();

                if(input.keyState(SDLK_p).firstPress)
                    std::cout << "Cam pos:" << hmdCamera.cullingView().camera.pos << std::endl;
//                else if(input.keyState(SDLK_n).firstPress)
//                {
//                    portalManager.setEnableEdge(false, scene1, portal1);
//                    portalManager.setEnableEdge(false, scene2, portal2);
//                }
//                else if(input.keyState(SDLK_m).firstPress)
//                {
//                    portalManager.setEnableEdge(true, scene1, portal1);
//                    portalManager.setEnableEdge(true, scene2, portal2);
//                }

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
