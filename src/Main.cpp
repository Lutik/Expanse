 
#include "SDL_Utils.h"

#include "Application.h"
#include "Utils/Logger/Logger.h"

#include "backends/imgui_impl_sdl.h"

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

    // Init ImGui bindings
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window.window, /*unused*/nullptr);


    Expanse::Point wnd_size;
    SDL_GetWindowSize(window.window, &wnd_size.x, &wnd_size.y);
    Expanse::Point fb_size;
    SDL_GL_GetDrawableSize(window.window, &fb_size.x, &fb_size.y);

    Application app;
    app.Init(wnd_size, fb_size);

    
    


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
            app.ProcessSystemEvent(e);
        }

        app.Tick();

        if (app.ReadyToQuit()) {
            quit = true;
        }

        window.SwapBuffers();
    }

	return 0;
}