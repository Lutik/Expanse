 
#include "SDL_Utils.h"

#include <GL/glew.h>

void InitRender()
{
    glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
}

void Draw()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

int main(int argc, char* args[])
{
	// Init SDL
    SDL::System sdl;
	if (!sdl)
		return 0;

	// Create main window
	SDL::WindowOpenGL window{ "Expanse", 1440, 810 };
	if (!window)
		return 0;

    InitRender();

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

        Draw();

        window.SwapBuffers();
    }

	return 0;
}