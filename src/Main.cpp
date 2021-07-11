 
#include "SDL_Utils.h"

#include "Application.h"
#include "Utils/Logger/Logger.h"

constexpr int WindowWidth = 1440;
constexpr int WindowHeight = 810;

SDL::WindowOpenGL CreateWindow()
{
    const SDL::GLContextParams params{
        .version = {3, 3},
        .swap_interval = 1,
    };
    return SDL::WindowOpenGL{ "Expanse", WindowWidth, WindowHeight, params };
}

int main(int argc, char* args[])
{
    using namespace Expanse;

    Log::init();

	// Init SDL
    SDL::System sdl;
	if (!sdl)
		return 0;

	// Create main window
    auto window = CreateWindow();
	if (!window)
		return 0;

    Application app;
    app.Init();

    // Main loop
    bool quit = false;
    while (!quit)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        app.Tick();

        window.SwapBuffers();
    }

	return 0;
}