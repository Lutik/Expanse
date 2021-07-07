
#include "SDL_Utils.h"

#include <SDL_image.h>
#include <GL/glew.h>

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

	Window::Window(const char* title, int width, int height, Uint32 flags)
	{
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | flags);		
	}

	Window::~Window()
	{	
		SDL_DestroyWindow(window);
	}

	WindowOpenGL::WindowOpenGL(const char* title, int width, int height, const OpenGLWindowParams& params)
		: Window(title, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL)
	{
		if (window)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, params.majorVersion);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, params.minorVersion);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

			context = SDL_GL_CreateContext(window);
			if (context)
			{
				SDL_GL_SetSwapInterval(params.swapInterval); // use VSync
			}
		}
	}

	WindowOpenGL::~WindowOpenGL()
	{
		SDL_GL_DeleteContext(context);
	}

	void WindowOpenGL::SwapBuffers()
	{
		SDL_GL_SwapWindow(window);
	}
}