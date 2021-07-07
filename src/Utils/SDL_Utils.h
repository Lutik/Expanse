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

	class Window
	{
	public:
		Window(const char* title, int width, int height, Uint32 Flags = 0);
		~Window();

		Window(const Window& other) = delete;
		Window& operator=(const Window& other) = delete;

		operator bool() const { return window != nullptr; }

	protected:
		SDL_Window* window = nullptr;		
	};

	struct OpenGLWindowParams
	{
		int majorVersion = 3;
		int minorVersion = 1;
		int swapInterval = 1;
	};
	class WindowOpenGL : public Window
	{
	public:
		WindowOpenGL(const char* title, int width, int height, const OpenGLWindowParams& params = {});
		~WindowOpenGL();

		operator bool() const { return window && context; }

		void SwapBuffers();
	protected:
		SDL_GLContext context;
	};
}