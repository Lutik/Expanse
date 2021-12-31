#pragma once

#include "Render/IRenderer.h"

#include "Utils/Timers.h"

#include "Game/World.h"
#include "Game/ISystem.h"
#include "GUI/ImGuiRenderer.h"
#include "Input/Input.h"
#include "AppState.h"

#include <SDL.h>

namespace Expanse
{
    class Application
    {
    public:
        void Init(Point window_size, Point framebuffer_size);

        void Tick();

        bool ReadyToQuit() const { return ready_to_quit; }

        void ProcessSystemEvent(const SDL_Event& evt);
    private:

        // declaration order is important for correct destruction order
        std::unique_ptr<Render::IRenderer> renderer;
        std::unique_ptr<ImGuiRenderer> gui_renderer;
        std::unique_ptr<IAppState> app_state;
    
        Timer timer;
        bool ready_to_quit = false;

        // Handles system events to update input state
        void ProcessInputEvent(const SDL_Event& event);

        void SwitchToState(AppState state);
    };

    /*********************************************************/

    class GameScreen final : public IAppState
    {
    public:
        GameScreen(Render::IRenderer* renderer, ImGuiRenderer* gui_render);

        void Update(float dt) override;

    private:
        Game::World world;
        std::unique_ptr<Game::SystemCollection> systems;

        void InitSystems(Render::IRenderer* renderer, ImGuiRenderer* gui_render);
    };

    /*********************************************************/

    class MainMenu final : public IAppState
    {
    public:
        MainMenu(Render::IRenderer* renderer, ImGuiRenderer* gui_renderer);

        void Update(float dt) override;
    private:
        ImGuiRenderer* gui_render = nullptr;

        void ShowMainMenu();
    };
}