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
		Window(const char* title, int width, int height);
		~Window();

		Window(const Window& other) = delete;
		Window& operator=(const Window& other) = delete;

		operator bool() const { return window != nullptr; }

	protected:
		SDL_Window* window = nullptr;
	};
}