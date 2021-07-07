
#include "SDL_Utils.h"

#include <SDL_image.h>

namespace SDL
{
	System::System()
	{
		// Init SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			return;

		// Init SDL_image
		constexpr int RequiredFlags = IMG_INIT_PNG;
		const int initFlags = IMG_Init(RequiredFlags);
		if ((initFlags & RequiredFlags) != RequiredFlags)
			return;

		init = true;
	}

	System::~System()
	{
		IMG_Quit();
		SDL_Quit();
	}


	Window::Window(const char* title, int width, int height)
	{
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	}

	Window::~Window()
	{
		SDL_DestroyWindow(window);
	}
}