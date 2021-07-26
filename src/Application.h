#pragma once

#include "Render/IRenderer.h"

#include "Utils/Timers.h"

#include "Game/World.h"
#include "Game/ISystem.h"

#include <SDL.h>

#include "GUI/ImGuiRenderer.h"

namespace Expanse
{
    class Application
    {
    public:
        Application();

        void Init(Point window_size, Point framebuffer_size);

        void Tick();

        void ProcessSystemEvent(const SDL_Event& evt);
    private:
        std::unique_ptr<Render::IRenderer> renderer;
        Timer timer;
        
        Game::World world;
        std::unique_ptr<Game::SystemCollection> systems;


        void ImGuiFrame(float dt);

        std::unique_ptr<ImGuiRenderer> imgui_render;
    };
}