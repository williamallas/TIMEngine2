
#include "SDLInputManager.h"
#include "DebugCamera.h"
#include "SDLTimer.h"
#include <SDL_image.h>
#include "SDLTextureLoader.h"

#include "Rand.h"
#include "renderer/ShaderCompiler.h"
#include "resource/MeshLoader.h"

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

    LOG(openGL.strHardward());
{
    MeshLoader::LoadedMeshData torData = MeshLoader::importObj("tor.obj");
    MeshLoader::LoadedMeshData sphereData = MeshLoader::importObj("uv_sphere.obj");
    MeshLoader::LoadedMeshData cubeData = MeshLoader::importObj("cube_uv.obj");
    MeshLoader::LoadedMeshData monkData = MeshLoader::importObj("monkey.obj");

    std::cout << "OBJ loaded ok\n";
    std::cout << "nbVertex tor : " << torData.nbVertex << std::endl;
    std::cout << "nbVertex sphere : " << sphereData.nbVertex << std::endl;
    std::cout << "nbVertex cube : " << cubeData.nbVertex << std::endl;
    std::cout << "nbVertex monk : " << monkData.nbVertex << std::endl;

    MeshBuffers* torMesh = MeshLoader::createMeshBuffers(torData, vertexBufferPool, indexBufferPool);
    MeshBuffers* sphereMesh = MeshLoader::createMeshBuffers(sphereData, vertexBufferPool, indexBufferPool);
    MeshBuffers* cubeMesh = MeshLoader::createMeshBuffers(cubeData, vertexBufferPool, indexBufferPool);
    MeshBuffers* monkMesh = MeshLoader::createMeshBuffers(monkData, vertexBufferPool, indexBufferPool);
    torData.clear();
    sphereData.clear();
    cubeData.clear();
    monkData.clear();

    Geometry geometry[3];
    geometry[0] = Geometry(torMesh);
    geometry[1] = Geometry(sphereMesh);
    geometry[2] = Geometry(cubeMesh);

    TextureLoader::ImageFormat imgLoaded;
    SDLTextureLoader textureLoader;
    ubyte* texData = textureLoader.loadImage("grass_pure.png", imgLoaded);
    std::cout << "sol.png loaded : " << imgLoaded.size << std::endl;
    renderer::Texture::GenTexParam texParam;
    texParam.size = uivec3(imgLoaded.size,0);
    texParam.format = renderer::Texture::Format::RGBA8;
    texParam.nbLevels = -1;
    texParam.repeat=true; texParam.linear=true; texParam.trilinear=true;
    renderer::Texture* texTest = renderer::Texture::genTexture2D(texParam, texData, imgLoaded.nbComponent);
    texTest->makeBindless();
    interface::Texture solTex(texTest);

    texData = textureLoader.loadImage("grass_pure_NRM.png", imgLoaded);
    std::cout << "sol_NRM.png loaded : " << imgLoaded.size << std::endl;
    texParam.size = uivec3(imgLoaded.size,0);
    texParam.format = renderer::Texture::Format::RGBA8;
    texParam.nbLevels = -1;
    texTest = renderer::Texture::genTexture2D(texParam, texData, imgLoaded.nbComponent);
    texTest->makeBindless();
    interface::Texture solNrmTex(texTest);

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
        elem.setMetallic(1);
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
    sceneEntity.globalLight.dirLights.push_back({vec3(0,0,-1), vec4(1,1,1,1), true});

    Pipeline::SceneView sceneView;
    sceneView.camera.ratio = RES_X/RES_Y;
    sceneView.camera.clipDist = {.1,1000};

    mat4 mat = mat4::Scale(vec3(100,100,1));
    mat.setTranslation({0,0,-1});
    MeshInstance& inst = sceneEntity.scene.add<MeshInstance>(mesh[2], mat);

    Mesh tmp = inst.mesh();
    tmp.element(0).setColor({0.5,0.5,0.5,1});
    tmp.element(0).setRougness(0.7);
    tmp.element(0).setMetallic(0);
    tmp.element(0).setSpecular(0.08);
    tmp.element(0).setTexture(solTex, 0);
    tmp.element(0).setTexture(solNrmTex, 1);
    inst.setMesh(tmp);

    for(int i=0 ; i<100 ; ++i)
        sceneEntity.scene.add<MeshInstance>(mesh[i%3], mat4::Translation({Rand::frand()*100-50,Rand::frand()*100-50,Rand::frand()*10}));

    Pipeline::DeferredRendererEntity& rendererEntity = pipeline.genDeferredRendererEntity({uint(RES_X),uint(RES_Y)});
    rendererEntity.envLightRenderer().setEnableGI(true);
    rendererEntity.envLightRenderer().setGlobalAmbient(vec4::construct(0));
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

    sceneEntity.globalLight.skybox = {skyboxes[3], processedSky[3]};

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
    dirLightCullingNode.setDepthMapResolution(2048);
    dirLightCullingNode.setShadowLightRange({50,100,400});

    pipeline::DirLightShadowNode& sunShadowNode = pipeline.createNode<pipeline::DirLightShadowNode>();
    sunShadowNode.setSceneView(dirLightView);
    sunShadowNode.addMeshInstanceCollector(dirLightCullingNode);
    sunShadowNode.setDepthMapResolution(2048);
    sunShadowNode.setShadowLightRange({50,100,400});

    rendererNode.setDirLightShadow(sunShadowNode, 0);
    rendererNode.addMeshInstanceCollector(meshCullingNode);

    onScreenRender.setBufferOutputNode(rendererNode.outputNode(0),0);

    int indexSkybox = 0;
    while(!input.keyState(SDLK_ESCAPE).pressed)
    {
        SDLTimer timer;
        input.getEvent();

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
}
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
                                      SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL  /*| SDL_WINDOW_FULLSCREEN*/);
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
