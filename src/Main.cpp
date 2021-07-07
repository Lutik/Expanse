 
#include "Utils/SDL_Utils.h"

int main(int argc, char* args[])
{
	// Init SDL
    SDL::System sdl;
	if (!sdl)
		return 0;

	// Create main window
	SDL::Window window{ "Expanse", 1440, 810 };
	if (!window)
		return 0;

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
    }

	return 0;
}