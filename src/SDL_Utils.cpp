
#include "SDL_Utils.h"

#include <SDL_image.h>
#include <GL/glew.h>

#include <utility>

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

	Window::Window(Window&& other)
	{
		window = std::exchange(other.window, nullptr);
	}

	Window& Window::operator=(Window&& other)
	{
		window = std::exchange(other.window, nullptr);
		return *this;
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

	WindowOpenGL::WindowOpenGL(WindowOpenGL&& other)
		: Window(std::move(other))
	{
		context = std::exchange(other.context, nullptr);
	}

	WindowOpenGL& WindowOpenGL::operator=(WindowOpenGL&& other)
	{
		Window::operator=(std::move(other));
		context = std::exchange(other.context, nullptr);
		return *this;
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