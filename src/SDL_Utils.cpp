
#include "SDL_Utils.h"

namespace SDL
{
	System::System()
	{
		// Init SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			return;

		init = true;
	}

	System::~System()
	{
		SDL_Quit();
	}

	/**********************************************************************************/

	Window::Window(const char* title, int width, int height, Uint32 flags)
	{
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | flags);		
	}

	Window::~Window()
	{
		if (window) {
			SDL_DestroyWindow(window);
		}
	}

	/**********************************************************************************/

	WindowOpenGL::WindowOpenGL(const char* title, int width, int height, const GLContextParams& params)
		: Window(title, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL)
	{
		if (window)
		{
			context = CreateContext(params);
		}
	}

	WindowOpenGL::~WindowOpenGL()
	{
		SDL_GL_DeleteContext(context);
	}

	SDL_GLContext WindowOpenGL::CreateContext(const GLContextParams& params)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, params.version.major);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, params.version.minor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, params.depth_size);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		SDL_GLContext gl_context = SDL_GL_CreateContext(window);
		if (gl_context)
		{
			SDL_GL_SetSwapInterval(params.swap_interval); // use VSync
		}

		return gl_context;
	}

	void WindowOpenGL::SwapBuffers()
	{
		SDL_GL_SwapWindow(window);
		
	}
}