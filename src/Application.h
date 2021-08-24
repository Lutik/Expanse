#pragma once

#include "Render/IRenderer.h"

#include "Utils/Timers.h"

#include "Game/World.h"
#include "Game/ISystem.h"
#include "Input/Input.h"

#include <SDL.h>

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

        Point window_size;


        // Handles system events to update input state
        void ProcessInputEvent(const SDL_Event& event);

        void InitSystems();
    };
}