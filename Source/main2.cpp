#include "MainHelper.h"

#include "OpenVR/OnHmdRenderer.h"
#include "OpenVR/HmdSceneView.h"
#include "OpenVR/VRDebugCamera.h"
#include "resource/AssetManager.h"
#include "MultiPromise.h"

#include "PortalGame/PortalGame.h"

#undef interface
using namespace tim::core;
using namespace tim::renderer;
using namespace tim::resource;
using namespace tim::interface;
using namespace tim;

int main(int argc, char* argv[])
{
    uint RES_X = 1512;
    uint RES_Y = 1680;

//    ConnectFourIA iaTest(true, 9);
//    do
//    {
//        iaTest.printGrid();
//        std::cout << "Score for you:" << iaTest.computeScore(true) << std::endl;
//        std::cout << "Index col (1-7):";
//        int in=0;
//        std::cin >> in;

//        iaTest.humanPlay(in-1);

//        if(iaTest.checkWinner() != ConnectFourIA::EMPTY)
//            break;

//        iaTest.computerPlay();
//        std::cout << std::endl;
//    }
//    while(iaTest.checkWinner() == ConnectFourIA::EMPTY);

//    iaTest.printGrid();
//    std::cout << "Winner is " << iaTest.checkWinner() << std::endl;
//    getchar();
//    exit(8);

    std::cout << "Enter the size of the room in meters (between 2 and 3) :";
    float meters=3; std::cin >> meters; std::cin.clear();
    meters = std::min(std::max(2.f,meters), 3.f);

    std::cout << "1. Tutorial\n2. Forest\n3. Maze\n4. Sacred Groove\n5. Ocean" << std::endl;
    std::cout <<std::endl<< "Enter the ID of the level you want to start in :";
    int indexLevel=1;
    std::cin >> indexLevel;
    indexLevel = std::min(std::max(1, indexLevel), 5);

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
            pipelineParam.shadowResolution = 2048;
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

            HmdSceneView hmdCamera(110, ratio, 500);
            hmdCamera.setScaleRoom(3 / meters);

            pipeline.setStereoView(hmdCamera.cullingView(), hmdCamera.eyeView(0), hmdCamera.eyeView(1), 0);

            VRDebugCamera debugCamera(&input, vec3(300,300,300));

            /* physic and setup */
            BulletEngine physEngine;
            ThreadPool threadPool(4);

            MultipleSceneHelper portalManager(pipelineParam, pipeline);
            portalManager.setResolution({RES_X,RES_Y});
            portalManager.setView(hmdCamera.cullingView());
            portalManager.setStereoView(hmdCamera.eyeView(0), hmdCamera.eyeView(1));
            portalManager.extendPipeline(NB_MAX_PIPELINE);

            switch(indexLevel)
            {
            case 1: indexLevel = 0; break;
            case 2: indexLevel = 2; break;
            case 3: indexLevel = 4; break;
            case 4: indexLevel = 5; break;
            case 5: indexLevel = 9; break;
            default: indexLevel = 0; break;
            }

            PortalGame portalGame(physEngine, portalManager, hmdCamera, hmdDevice, indexLevel);

            if (hmdDevice.isInit()) hmdDevice.sync();

            float freqphys = 500;
            btSphereShape ballShape(0.075);

            float timeElapsed = 0, totalTime = 0;
            float scaleRoom = 1;

			while (!input.keyState(SDLK_ESCAPE).pressed)
			{
                SDLTimer timer;

                input.getEvent();
                /*PROFILE("Wait thread")*/ threadPool.wait();

                if(input.keyState(SDLK_x).firstPress)
                {
                    scaleRoom -= 0.05;
                    hmdCamera.setScaleRoom(scaleRoom);
                    std::cout << "Scale room:" << scaleRoom << std::endl;
                }
                if(input.keyState(SDLK_y).firstPress)
                {
                    scaleRoom += 0.05;
                    hmdCamera.setScaleRoom(scaleRoom);
                    std::cout << "Scale room:" << scaleRoom << std::endl;
                }

//                if(input.keyState(SDLK_n).firstPress && !input.keyState(SDLK_r).pressed)
//                {
//                    portalGame.controllers().DAMPING -= 10;
//                    std::cout << "DAMPING:" << portalGame.controllers().DAMPING << std::endl;
//                }
//                if(input.keyState(SDLK_m).firstPress && !input.keyState(SDLK_r).pressed)
//                {
//                    portalGame.controllers().DAMPING += 10;
//                    std::cout << "DAMPING:" << portalGame.controllers().DAMPING << std::endl;
//                }

//                if(input.keyState(SDLK_k).firstPress && !input.keyState(SDLK_r).pressed)
//                {
//                    portalGame.controllers().STRENGTH -= 50;
//                    std::cout << "STRENGTH:" << portalGame.controllers().STRENGTH << std::endl;
//                }
//                if(input.keyState(SDLK_l).firstPress && !input.keyState(SDLK_r).pressed)
//                {
//                    portalGame.controllers().STRENGTH += 50;
//                    std::cout << "STRENGTH:" << portalGame.controllers().STRENGTH << std::endl;
//                }

//                if(input.keyState(SDLK_n).firstPress && input.keyState(SDLK_r).pressed)
//                {
//                    portalGame.controllers().DAMPING_R -= 0.1;
//                    std::cout << "DAMPING_R:" << portalGame.controllers().DAMPING_R << std::endl;
//                }
//                if(input.keyState(SDLK_m).firstPress && input.keyState(SDLK_r).pressed)
//                {
//                    portalGame.controllers().DAMPING_R += 0.1;
//                    std::cout << "DAMPING_R:" << portalGame.controllers().DAMPING_R << std::endl;
//                }

//                if(input.keyState(SDLK_k).firstPress && input.keyState(SDLK_r).pressed)
//                {
//                    portalGame.controllers().STRENGTH_R -= 1;
//                    std::cout << "STRENGTH_R:" << portalGame.controllers().STRENGTH_R << std::endl;
//                }
//                if(input.keyState(SDLK_l).firstPress && input.keyState(SDLK_r).pressed)
//                {
//                    portalGame.controllers().STRENGTH_R += 1;
//                    std::cout << "STRENGTH_R:" << portalGame.controllers().STRENGTH_R << std::endl;
//                }

//                if(input.keyState(SDLK_p).firstPress)
//                {
//                    freqphys += 10;
//                    std::cout << "PHYSIQUE_FREQ:" << freqphys << std::endl;
//                }
//                if(input.keyState(SDLK_o).firstPress)
//                {
//                    freqphys -= 10;
//                    std::cout << "PHYSIQUE_FREQ:" << freqphys << std::endl;
//                }

                if(input.keyState(SDLK_b).firstPress)
                {
                    portalGame.popBoxDebug();
                }
                if(input.keyState(SDLK_v).firstPress)
                {
                    portalGame.levelSystem().callDebug();
                }

//                if(input.keyState(SDLK_y).firstPress)
//                {
//                    hmdNode->setInvertEyes(false);
//                }
//                else if(input.keyState(SDLK_x).firstPress)
//                {
//                    hmdNode->setInvertEyes(true);
//                }

                /*********************/
                /******* Flush *******/
                /*********************/

                auto futur_updateGame = threadPool.schedule_trace([&]()
                {
                    /*PROFILE("Update game")*/ portalGame.update(timeElapsed);
                    //endUpdateGamePromise.set_value();
                    return true;
                });

                auto futur_prepare = threadPool.schedule_trace([&]()
                {
                    futur_updateGame.wait();
                    /*PROFILE("Pipeline prepare")*/ pipeline.pipeline()->prepare();
                    return true;
                });

                int sceneIndex = portalGame.curSceneIndex();
                int nbLevel = portalGame.levelSystem().nbLevels();
                threadPool.schedule([&physEngine, timeElapsed, sceneIndex, freqphys, nbLevel, &futur_updateGame]()
                {
                    futur_updateGame.wait();
                    for(int i=0 ; i<nbLevel ; ++i)
                        if(physEngine.dynamicsWorld[i])
                            physEngine.dynamicsWorld[i]->stepSimulation(std::min(timeElapsed, 1.f/60), 20, 1 / freqphys);
               });

                if (hmdDevice.isInit())
                    hmdDevice.update(true);
                else
                    debugCamera.update(timeElapsed);

                futur_prepare.wait();

                if (hmdDevice.isInit())
                    hmdCamera.update(hmdDevice);
                else
                    hmdCamera.update(debugCamera, ratio);

                portalManager.updateCameras();

                portalGame.setDebugControllerPose(debugCamera.pos() + debugCamera.dir()*0.5);

                /********************/
                /******* Draw *******/
                /********************/

                /*PROFILE("Pipeline render")*/ pipeline.pipeline()->render();
                /*PROFILE("Swapbuffer")*/ swapBuffer();

                /*PROFILE("GL_Assert")*/ GL_ASSERT();

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
