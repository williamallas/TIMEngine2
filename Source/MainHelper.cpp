#include "MainHelper.h"
#include "renderer/GLState.h"
#include <SDL_image.h>

#undef interface
using namespace tim;

SDL_Window *pWindow;
SDL_GLContext contexteOpenGL;

void initContextSDL()
{
	/* Initialisation simple */
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		LOG("Echec de l'initialisation de la SDL\n"); //out(SDL_GetError()); //out("\n");
		system("pause");
		return;
	}

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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
    renderer::openGL.setViewPort({ 0,0 }, { WIN_RES_X, WIN_RES_Y });

	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
}

void swapBuffer()
{
    SDL_GL_SwapWindow(pWindow);
}

void delContextSDL()
{
	IMG_Quit();
	SDL_GL_DeleteContext(contexteOpenGL);
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}
