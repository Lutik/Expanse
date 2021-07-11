 
#include "SDL_Utils.h"

#include <GL/glew.h>

#include "Render/IRenderer.h"
#include "Utils/Logger/Logger.h"

#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/VertexArrayObject.h"

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

namespace Expanse
{
    class Application
    {
    public:
        void Init()
        {
            renderer = Render::CreateOpenGLRenderer();
            shader = { "content/shaders/basic.vs", "content/shaders/basic.fs" };

            const std::vector<Render::VertexP2> verts = {
                {{-1.0f, -1.0f}}, {{1.0f, -1.0f}}, {{0.0f, 1.0f}}
            };
            vao.SetVertices(verts);
        }

        void Tick()
        {
            renderer->ClearFrame();

            shader.Bind();
            vao.Draw();
        }

    private:
        std::unique_ptr<Render::IRenderer> renderer;

        Render::GL::ShaderProgram shader;
        Render::GL::VertexArray vao;
    };
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