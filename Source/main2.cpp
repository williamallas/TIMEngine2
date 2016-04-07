
#include "SDLInputManager.h"
#include "DebugCamera.h"
#include "SDLTimer.h"
#include <SDL_image.h>
#include "SDLTextureLoader.h"

#include "Rand.h"
#include "renderer/ShaderCompiler.h"
#include "resource/MeshLoader.h"
#include "resource/AssetManager.h"
#include "bullet/BulletEngine.h"

#include "interface/pipeline/pipeline.h"
#include "interface/Pipeline.h"

using namespace tim::core;
using namespace tim::renderer;
using namespace tim::resource;
using namespace tim::interface;
using namespace tim;

SDL_Window *pWindow;
SDL_GLContext contexteOpenGL;
void initContextSDL();
void delContextSDL();
#define RES_X 1920.f
#define RES_Y 1080.f

template <int I, int N=50>
float countTime(float time)
{
    static uint counter=0;
    static float totalTime=0;
    static float fps=0;

    counter ++;

    totalTime += time;
    if(counter % N == 0)
    {
        fps = totalTime/N;
        counter = 0;
        totalTime = 0;
        return fps;
    }
    return -fps;
}

int main(int, char**)
{
    tim::core::init();
{
    initContextSDL();
    tim::renderer::init();
    resource::textureLoader = new SDLTextureLoader;

    LOG(openGL.strHardward());
{
    MeshLoader::LoadedMeshData objSphere = MeshLoader::importObj("uv_sphere.obj");
    MeshLoader::exportTim(objSphere, "sphere.tim");

    BulletEngine physEngine;
    btBoxShape boxShape(btVector3(0.5,0.5,0.5));
    btStaticPlaneShape groundShape(btVector3(0,0,1), 0);
    BulletObject obj(mat4::IDENTITY(), &groundShape);
    physEngine.addObject(&obj);
    obj.body()->setFriction(1000);

    Geometry geometry[3];

    geometry[0] = AssetManager<Geometry>::instance().load<false>("tor.obj").value();
    geometry[1] = AssetManager<Geometry>::instance().load<false>("sphere.tim").value();
    geometry[2] = AssetManager<Geometry>::instance().load<false>("cube_uv.obj").value();

    renderer::Texture::GenTexParam texParam;
    texParam.format = renderer::Texture::Format::RGBA8;
    texParam.nbLevels = -1;
    texParam.repeat=true; texParam.linear=true; texParam.trilinear=true;

    interface::Texture solTexture = AssetManager<interface::Texture>::instance().load<false>("grass_pure.png", texParam).value();
    interface::Texture solNrmTexture = AssetManager<interface::Texture>::instance().load<false>("grass_pure_NRM.png", texParam).value();

    ShaderCompiler vShader(ShaderCompiler::VERTEX_SHADER), pShader(ShaderCompiler::PIXEL_SHADER);
    vShader.setSource(StringUtils::readFile("shader/gBufferPass.vert"));
    pShader.setSource(StringUtils::readFile("shader/gBufferPass.frag"));

    auto optShader = Shader::combine(vShader.compile({}), pShader.compile({}));

    if(!optShader.hasValue())
    {
        LOG("Vertex shader error:\n", vShader.error());
        LOG("Pixel shader error:\n", pShader.error());
        LOG("Link erorr:", Shader::lastLinkError());
        return 0;
    }

    Mesh mesh[3];
    for(int i=0 ; i<3 ; ++i)
    {
        Mesh::Element elem;
        elem.drawState().setShader(optShader.value());
        elem.setGeometry(geometry[i]);
        elem.setRougness(0.5);
        elem.setMetallic(0);
        elem.setSpecular(0.1);
        elem.setColor(vec4(vec3(Rand::frand(), Rand::frand(), Rand::frand()).saturated(),1));

        mesh[i].addElement(elem);
    }


    SDLInputManager input;
    float timeElapsed=0, totalTime=0;
    //int indexSkybox=0;

    DebugCamera freeFly(&input);

    /* Example of main */

    Pipeline pipeline;

    /* Pipeline entity */
    Pipeline::SceneEntity<SimpleScene> sceneEntity;
    sceneEntity.globalLight.dirLights.push_back({vec3(0,0,-1), vec4::construct(0.5), true});

    Pipeline::SceneView sceneView;
    sceneView.camera.ratio = RES_X/RES_Y;
    sceneView.camera.clipDist = {.1,1000};

    mat4 mat = mat4::Scale(vec3(100,100,1));
    mat.setTranslation({0,0,-0.5});
    MeshInstance& inst = sceneEntity.scene.add<MeshInstance>(mesh[2], mat);

    Mesh tmp = inst.mesh();
    tmp.element(0).setColor({0.5,0.5,0.5,1});
    tmp.element(0).setRougness(0.7);
    tmp.element(0).setMetallic(0);
    tmp.element(0).setSpecular(0.08);
    tmp.element(0).setTexture(solTexture, 0);
    tmp.element(0).setTexture(solNrmTexture, 1);
    inst.setMesh(tmp);

    for(int i=0 ; i<100 ; ++i)
        sceneEntity.scene.add<MeshInstance>(mesh[i%2], mat4::Translation({Rand::frand()*100-50,Rand::frand()*100-50,Rand::frand()*10}));

    vector<std::shared_ptr<BulletObject>> physObj(10000);

    int index=0;
    for(int i=0 ; i<3 ; ++i)
        for(int j=0 ; j<3 ; ++j)
            for(int k=0 ; k<100 ; ++k)
    {
        MeshInstance& m = sceneEntity.scene.add<MeshInstance>(mesh[2], mat4::Translation({i*1.01,j*1.01,k*1.01+10}));
        physObj[index] = std::shared_ptr<BulletObject>(new BulletObject(new SceneMotionState<MeshInstance>(m), &boxShape, 1));
        physObj[index].get()->body()->setFriction(1000);
        physEngine.addObject(physObj[index++].get());
    }

    for(int i=0 ; i<50 ; ++i)
    {
        LightInstance& light = sceneEntity.scene.add<LightInstance>();
        light.setPosition({Rand::frand()*100-50,Rand::frand()*100-50,Rand::frand()*10});
        light.setRadius(30);
        light.setPower(2);
        light.setColor({1,1,1,1});
    }

    Pipeline::DeferredRendererEntity& rendererEntity = pipeline.genDeferredRendererEntity({uint(RES_X),uint(RES_Y)});
    rendererEntity.envLightRenderer().setEnableGI(false);
    rendererEntity.envLightRenderer().setGlobalAmbient(vec4::construct(0.1));
    //rendererEntity.

    /* Load skybox */
    vector<renderer::Texture*> skyboxes(4);
    vector<renderer::Texture*> processedSky(skyboxes.size());
    {
        TextureLoader::ImageFormat formatTex;

        for(size_t i=0 ; i<skyboxes.size() ; ++i)
        {
            vector<std::string> imgSkybox(6);
            std::string pre = "skybox/simple" + StringUtils(i+1).str() + "/";
            imgSkybox[0] = pre+"x.png";
            imgSkybox[1] = pre+"nx.png";
            imgSkybox[2] = pre+"y.png";
            imgSkybox[3] = pre+"ny.png";
            imgSkybox[4] = pre+"z.png";
            imgSkybox[5] = pre+"nz.png";
            vector<ubyte*> dataSkybox = SDLTextureLoader().loadImageCube(imgSkybox, formatTex);

            renderer::Texture::GenTexParam skyboxParam;
            skyboxParam.format = renderer::Texture::RGBA8;
            skyboxParam.nbLevels = 1;
            skyboxParam.size = uivec3(formatTex.size,0);
            skyboxes[i] = renderer::Texture::genTextureCube(skyboxParam, dataSkybox, 4);
            processedSky[i] = IndirectLightRenderer::processSkybox(skyboxes[i],
                                                                   rendererEntity.envLightRenderer().processSkyboxShader());
            for(uint j=0 ; j<dataSkybox.size() ; ++j)
                delete[] dataSkybox[j];
        }
    }

    //sceneEntity.globalLight.skybox = {skyboxes[3], processedSky[3]};

    /* build the graph */
    pipeline::OnScreenRenderer& onScreenRender = pipeline.createNode<pipeline::OnScreenRenderer>();
    pipeline.setOutputNode(onScreenRender);

    pipeline::SimpleSceneMeshCullingNode& meshCullingNode = pipeline.createNode<pipeline::SimpleSceneMeshCullingNode>();
    pipeline::DeferredRendererNode& rendererNode = pipeline.createNode<pipeline::DeferredRendererNode>();

    meshCullingNode.setScene(sceneEntity);
    meshCullingNode.setSceneView(sceneView);
    rendererNode.setSceneView(sceneView);
    rendererNode.setRendererEntity(rendererEntity);
    rendererNode.setGlobalLight(sceneEntity.globalLight);

    Pipeline::SceneView dirLightView;
    dirLightView.dirLightView.lightDir = vec3(0,0.7,-1);

    pipeline::DirLightCullingNode<SimpleScene>& dirLightCullingNode =
            pipeline.createNode<pipeline::DirLightCullingNode<SimpleScene>>();

    dirLightCullingNode.setScene(sceneEntity);
    dirLightCullingNode.setSceneView(dirLightView);
    dirLightCullingNode.setDepthMapResolution(4096);
    dirLightCullingNode.setShadowLightRange({20,100,300,800});

    pipeline::DirLightShadowNode& sunShadowNode = pipeline.createNode<pipeline::DirLightShadowNode>();
    sunShadowNode.setSceneView(dirLightView);
    sunShadowNode.addMeshInstanceCollector(dirLightCullingNode);
    sunShadowNode.setDepthMapResolution(4096);
    sunShadowNode.setShadowLightRange({20,100,300,800});

    rendererNode.setDirLightShadow(sunShadowNode, 0);
    rendererNode.addMeshInstanceCollector(meshCullingNode);

    pipeline::SimpleSceneLightCullingNode& lightCullingNode = pipeline.createNode<pipeline::SimpleSceneLightCullingNode>();
    lightCullingNode.setScene(sceneEntity);
    lightCullingNode.setSceneView(sceneView);
    rendererNode.addLightInstanceCollector(lightCullingNode);

    onScreenRender.setBufferOutputNode(rendererNode.outputNode(0),0);

    int indexSkybox = 0;
    while(!input.keyState(SDLK_ESCAPE).pressed)
    {
        SDLTimer timer;
        input.getEvent();
        if(input.keyState(SDLK_n).pressed)
             physEngine.dynamicsWorld->stepSimulation(timeElapsed, 10,0.01);

        freeFly.update(timeElapsed, sceneView.camera);
        dirLightView.dirLightView.camPos = sceneView.camera.pos;

        pipeline.prepare();
        pipeline.render();

        SDL_GL_SwapWindow(pWindow);
        GL_ASSERT();

        if(input.keyState(SDLK_RIGHT).firstPress)
        {
            indexSkybox = (indexSkybox+1)%skyboxes.size();
            sceneEntity.globalLight.skybox = {skyboxes[indexSkybox], processedSky[indexSkybox]};
        }

        if(input.keyState(SDLK_m).pressed)
        {
        int index=0;
        for(int i=0 ; i<3 ; ++i)
            for(int j=0 ; j<3 ; ++j)
                for(int k=0 ; k<100 ; ++k)
        {

              physObj[index].get()->body()->applyForce(btVector3((Rand::frand()-0.5)*500, (Rand::frand()-0.5)*500, (Rand::frand()-0.5)*500), btVector3(0,0,0));
              index++;

        }
        }

        if(input.keyState(SDLK_l).pressed)
        {
        int index=0;
        for(int i=0 ; i<3 ; ++i)
            for(int j=0 ; j<3 ; ++j)
                for(int k=0 ; k<100 ; ++k)
        {

              physObj[index].get()->body()->applyForce(-physObj[index].get()->body()->getWorldTransform().getOrigin().normalized()*100, btVector3(0,0,0));
              index++;

        }
        }

        timeElapsed = timer.elapsed()*0.001;
        totalTime += timeElapsed;
        {
            float fps=countTime<0>(timeElapsed);
            if(fps>0)
            {
                std::cout << "Fps:" << 1.f/fps << "  Ms:" << 1000*fps<<std::endl;
            }
        }
    }

    /** Close context **/
    delete optShader.value();
    for(uint i=0 ; i<processedSky.size() ; ++i)
    {
        delete processedSky[i];
        delete skyboxes[i];
    }

    AssetManager<Geometry>::freeInstance();
    AssetManager<interface::Texture>::freeInstance();
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

    uint x=uint(RES_X), y=uint(RES_Y);
    pWindow = SDL_CreateWindow("SDL2",SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      x,y,
                                      SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL  | SDL_WINDOW_FULLSCREEN);
    contexteOpenGL = SDL_GL_CreateContext(pWindow);

    //SDL_ShowCursor(SDL_DISABLE);
    //SDL_SetWindowGrab(pWindow, SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    if(contexteOpenGL == 0)
    {
        LOG(SDL_GetError(),"\n");
        system("pause");
        return;
    }
    openGL.setViewPort({0,0},{x,y});

    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
}

void delContextSDL()
{
    IMG_Quit();
    SDL_GL_DeleteContext(contexteOpenGL);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
}
