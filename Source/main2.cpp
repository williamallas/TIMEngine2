#include "MainHelper.h"

#include "OpenVR/OnHmdRenderer.h"
#include "OpenVR/HmdSceneView.h"
#include "OpenVR/VRDebugCamera.h"
#include "resource/AssetManager.h"

#include "PortalGame/PortalGame.h"

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
            SDLInputManager input;

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
            pipelineParam.usePointLight = true;
            pipelineParam.shadowCascad = vector<float>({5, 25});
            pipelineParam.shadowResolution = 1024;
            pipelineParam.useSSReflexion = false;
            pipelineParam.usePostSSReflexion = false;
            pipelineParam.useFxaa = true;

            OnHmdRenderer* hmdNode = new OnHmdRenderer;
            hmdNode->setDrawOnScreen(2);
            float ratio = float(RES_X)/RES_Y;
            hmdNode->setScreenResolution({WIN_RES_X, WIN_RES_Y});
            hmdNode->setShader(ShaderPool::instance().get("feedbackStereo"));

            pipeline.createStereoExtensible(*hmdNode, {RES_X,RES_Y}, pipelineParam);
            hmdNode->setVRDevice(&hmdDevice);

            HmdSceneView hmdCamera(110, ratio, 100);
            pipeline.setStereoView(hmdCamera.cullingView(), hmdCamera.eyeView(0), hmdCamera.eyeView(1), 0);

            VRDebugCamera debugCamera(&input, vec3(3,3,2));

            /* physic and setup */
            BulletEngine physEngine;
            ThreadPool threadPool(4);

            MultipleSceneHelper portalManager(pipelineParam, pipeline);
            portalManager.setResolution({RES_X,RES_Y});
            portalManager.setView(hmdCamera.cullingView());
            portalManager.setStereoView(hmdCamera.eyeView(0), hmdCamera.eyeView(1));

            PortalGame portalGame(physEngine, portalManager, hmdCamera, hmdDevice);

            if (hmdDevice.isInit()) hmdDevice.sync();

            float freqphys = 500;
            btSphereShape ballShape(0.075);

            float timeElapsed = 0, totalTime = 0;

			while (!input.keyState(SDLK_ESCAPE).pressed)
			{
				SDLTimer timer;

                input.getEvent();
                threadPool.wait();	

                if(input.keyState(SDLK_n).firstPress && !input.keyState(SDLK_r).pressed)
                {
                    portalGame.controllers().DAMPING -= 10;
                    std::cout << "DAMPING:" << portalGame.controllers().DAMPING << std::endl;
                }
                if(input.keyState(SDLK_m).firstPress && !input.keyState(SDLK_r).pressed)
                {
                    portalGame.controllers().DAMPING += 10;
                    std::cout << "DAMPING:" << portalGame.controllers().DAMPING << std::endl;
                }

                if(input.keyState(SDLK_k).firstPress && !input.keyState(SDLK_r).pressed)
                {
                    portalGame.controllers().STRENGTH -= 50;
                    std::cout << "STRENGTH:" << portalGame.controllers().STRENGTH << std::endl;
                }
                if(input.keyState(SDLK_l).firstPress && !input.keyState(SDLK_r).pressed)
                {
                    portalGame.controllers().STRENGTH += 50;
                    std::cout << "STRENGTH:" << portalGame.controllers().STRENGTH << std::endl;
                }


                if(input.keyState(SDLK_n).firstPress && input.keyState(SDLK_r).pressed)
                {
                    portalGame.controllers().DAMPING_R -= 0.1;
                    std::cout << "DAMPING_R:" << portalGame.controllers().DAMPING_R << std::endl;
                }
                if(input.keyState(SDLK_m).firstPress && input.keyState(SDLK_r).pressed)
                {
                    portalGame.controllers().DAMPING_R += 0.1;
                    std::cout << "DAMPING_R:" << portalGame.controllers().DAMPING_R << std::endl;
                }

                if(input.keyState(SDLK_k).firstPress && input.keyState(SDLK_r).pressed)
                {
                    portalGame.controllers().STRENGTH_R -= 1;
                    std::cout << "STRENGTH_R:" << portalGame.controllers().STRENGTH_R << std::endl;
                }
                if(input.keyState(SDLK_l).firstPress && input.keyState(SDLK_r).pressed)
                {
                    portalGame.controllers().STRENGTH_R += 1;
                    std::cout << "STRENGTH_R:" << portalGame.controllers().STRENGTH_R << std::endl;
                }

                if(input.keyState(SDLK_p).firstPress)
                {
                    freqphys += 10;
                    std::cout << "PHYSIQUE_FREQ:" << freqphys << std::endl;
                }
                if(input.keyState(SDLK_o).firstPress)
                {
                    freqphys -= 10;
                    std::cout << "PHYSIQUE_FREQ:" << freqphys << std::endl;
                }

                if(input.keyState(SDLK_b).firstPress)
                {
                    portalGame.popBoxDebug();
                }
                if(input.keyState(SDLK_v).firstPress)
                {
                    portalGame.levelSystem().callDebug();
                }


                /*********************/
                /******* Flush *******/
                /*********************/

                if (hmdDevice.isInit())
                {
                    hmdDevice.update(false);
                    hmdCamera.update(hmdDevice);
                }
                else
                {
                    debugCamera.update(timeElapsed);
                    hmdCamera.update(debugCamera, ratio);
                }

                portalGame.update(timeElapsed);
                portalGame.setDebugControllerPose(debugCamera.pos() + debugCamera.dir()*0.5);

                int sceneIndex = portalGame.curSceneIndex();
                int nbLevel = portalGame.levelSystem().nbLevels();
                threadPool.schedule([&physEngine, timeElapsed, sceneIndex, freqphys, nbLevel]()
                {
                    for(int i=0 ; i<nbLevel ; ++i)
                        if(physEngine.dynamicsWorld[i])
                            physEngine.dynamicsWorld[i]->stepSimulation(std::min(timeElapsed, 1.f/60), 20, 1 / freqphys);
               });

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
                    {
                        std::cout << "Fps:" << 1.f / fps << "  Ms:" << 1000 * fps << std::endl;
                        std::cout << debugCamera.pos() << std::endl << std::endl;
                    }
                   
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
