#pragma once

#include <SDL.h>

namespace SDL
{
	class System final
	{
	public:
		System();
		~System();

		operator bool() const { return init; }
	private:
		bool init = false;
	};

	/**********************************************************************************/

	class Window
	{
	public:
		Window(const char* title, int width, int height, Uint32 Flags = 0);
		~Window();

		Window(const Window& other) = delete;
		Window& operator=(const Window& other) = delete;
		Window(Window&& other);
		Window& operator=(Window&& other);

		operator bool() const { return window != nullptr; }

	protected:
		SDL_Window* window = nullptr;		
	};

	/**********************************************************************************/

	struct GLVersion
	{
		int major;
		int minor;
	};

	struct GLContextParams
	{
		GLVersion version = { 3, 3 };
		int swap_interval = 1;
		int depth_size = 0;
	};

	class WindowOpenGL : public Window
	{
	public:
		WindowOpenGL(const char* title, int width, int height, const GLContextParams& params = {});
		~WindowOpenGL();

		WindowOpenGL(const WindowOpenGL& other) = delete;
		WindowOpenGL& operator=(const WindowOpenGL& other) = delete;
		WindowOpenGL(WindowOpenGL&& other);
		WindowOpenGL& operator=(WindowOpenGL&& other);

		operator bool() const { return window && context; }

		void SwapBuffers();

	protected:
		SDL_GLContext context;

		SDL_GLContext CreateContext(const GLContextParams& params);
	};
}