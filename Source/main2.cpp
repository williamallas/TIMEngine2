#include "SDL.h"
#include "SDL_image.h"
#include "SDLInputManager.h"
#include "SDLTextureLoader.h"
#include "SDLTimer.h"

#include "renderer/renderer.h"
#include "core.h"
#include "Rand.h"
#include "RangeAllocator.h"
#include "renderer/BufferPool.h"
#include "renderer/MeshBuffers.h"
#include "renderer/Shader.h"
#include "renderer/ShaderCompiler.h"
#include "resource/MeshLoader.h"
#include "renderer/VAO.h"
#include "renderer/MeshRenderer.h"
#include "renderer/TiledLightRenderer.h"
#include "renderer/DirectionalLightRenderer.h"
#include "renderer/IndirectLightRenderer.h"
#include "DebugCamera.h"

using namespace tim::core;
using namespace tim::renderer;
using namespace tim::resource;
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

__stdcall void debugOut(GLenum , GLenum , GLuint , GLenum severity , GLsizei , const GLchar* msg, GLvoid*)
{
    if(severity == GL_DEBUG_SEVERITY_HIGH)
    {
        LOG("OpenGL debug: ", msg);
    }
}

int main(int, char**)
{
    tim::core::init();
{
    initContextSDL();
    tim::renderer::init();

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback( debugOut, NULL );

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
    MeshBuffers* meshs[4] = {torMesh, sphereMesh,cubeMesh,monkMesh};
    torData.clear();
    sphereData.clear();
    cubeData.clear();
    monkData.clear();

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
    Shader* shader = optShader.value();
    DrawState drawState;
    drawState.setShader(shader);

    FrameParameter frameState;
    MeshRenderer meshRenderer(frameState);
    meshRenderer.setDrawState(drawState);
    meshRenderer.bind();

    vector<MeshBuffers*> drawList(100);
    vector<mat4> models(100);
    vector<Material> materials(100);
    for(size_t i=0 ; i<drawList.size()/10 ;++i)
        for(size_t j=0 ; j<drawList.size()/10 ;++j)
    {
        drawList[i*10+j] = meshs[Rand::rand()%4];
        models[i*10+j] = mat4::Translation({j*3,i*3,0});
        models[i*10+j].transpose();
        materials[i*10+j].color = vec4(vec3(Rand::frand(), Rand::frand(), Rand::frand()).saturate(),1);
        materials[i*10+j].parameter = vec4(i*0.11,j*0.11,0.3,1);
    }

    vector<LightContextRenderer::Light> lights(1);
    lights[0].position = {2,0,0};
    lights[0].radius = 5;
    lights[0].power = 2;

//    {
//        float* dat = IndirectLightRenderer::computeBrdf(256);
//        std::ofstream outDat("shader/brdf_256.dat", std::ofstream::binary);
//        outDat.write((char*)dat, sizeof(float)*256*256*3);
//        delete[] dat;
//    }

    DeferredRenderer deferred({uint(RES_X),uint(RES_Y)}, frameState);
    LightContextRenderer lightContext(deferred, true);
    TiledLightRenderer lightRenderer(deferred, false);
    DirectionalLightRenderer dirLightRenderer(lightContext);
    IndirectLightRenderer indirectLight(lightContext);

    TextureLoader::ImageFormat formatTex;
    vector<Texture*> skyboxes(4);
    vector<Texture*> processedSky(skyboxes.size());
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

        Texture::GenTexParam skyboxParam;
        skyboxParam.format = Texture::RGBA8;
        skyboxParam.nbLevels = 1;
        skyboxParam.size = uivec3(formatTex.size,0);
        skyboxes[i] = Texture::genTextureCube(skyboxParam, dataSkybox, 4);
        processedSky[i] = IndirectLightRenderer::processSkybox(skyboxes[i], indirectLight.processSkyboxShader());
        for(uint j=0 ; j<dataSkybox.size() ; ++j)
            delete[] dataSkybox[j];
    }
    indirectLight.setSkybox(skyboxes[0], processedSky[0]);

//    {
//        SDLTimer timer;
//        indirectLight.setSkybox(skybox, IndirectLightRenderer::processSkybox(skybox, indirectLight.processSkyboxShader()));
//        std::cout << "Time to process skybox: " << timer.elapsed() << "ms\n";
//    }

    DrawState stateDrawQuad;
    stateDrawQuad.setCullFace(false);
    stateDrawQuad.setDepthTest(false);
    stateDrawQuad.setWriteDepth(false);
    stateDrawQuad.setShader(drawQuadShader);

    SDLInputManager input;
    float timeElapsed=0, totalTime=0;
    int indexSkybox=0;

    Camera camera;
    camera.clipDist = {0.3, 1000};
    camera.fov = 70;
    camera.ratio = 16/9.f;

    DebugCamera freeFly(&input);

    while(!input.keyState(SDLK_ESCAPE).pressed)
    {
        SDLTimer timer;
        input.getEvent();

        freeFly.update(timeElapsed, camera);
        frameState.setCamera(camera);
        frameState.setTime(totalTime, timeElapsed);

        deferred.frameBuffer().bind();

        openGL.clearDepth();
        openGL.clearColor(vec4(0));
        meshRenderer.draw(drawList,models,materials);

        lightContext.frameBuffer().bind();
        openGL.clearColor({0,0,0,1});

        vector<DirectionalLightRenderer::Light> dirLight(1);
        dirLight[0].color = vec3::construct(1);
        dirLight[0].direction = mat4::RotationZ(totalTime)*vec3(0.5,0.5,-1);

        //meshRenderer.bind();
        dirLightRenderer.draw(dirLight);
        indirectLight.draw();

        lightRenderer.draw(lights);

        openGL.bindFrameBuffer(0);

        stateDrawQuad.bind();
        openGL.bindTextureSampler(textureSampler[TextureMode::NoFilter], 0);
        if(input.keyState(SDLK_1).pressed)
            deferred.buffer(0)->bind(0);
        else if(input.keyState(SDLK_2).pressed)
            deferred.buffer(1)->bind(0);
        else if(input.keyState(SDLK_3).pressed)
            deferred.buffer(2)->bind(0);
        else if(input.keyState(SDLK_4).pressed)
            lightRenderer.buffer()->bind(0);
       else
            lightContext.buffer()->bind(0);

        quadMeshBuffers->draw(6, VertexMode::TRIANGLES, 1);

        openGL.finish();
        SDL_GL_SwapWindow(pWindow);
        GL_ASSERT();

        if(input.keyState(SDLK_RIGHT).firstPress)
        {
            indexSkybox = (indexSkybox+1)%skyboxes.size();
            indirectLight.setSkybox(skyboxes[indexSkybox], processedSky[indexSkybox]);
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
