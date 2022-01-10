#include "Application.h"

#include "backends/imgui_impl_sdl.h"
#include "GUI/ImGuiRenderer.h"
#include "GUI/LogWindow.h"
#include "GUI/DebugWindow.h"

#include "Render/SpriteBatch.h"
#include "Utils/Random.h"

#include "Game/Terrain/Systems/GenerateTerrain.h"
#include "Game/Terrain/Systems/StreamTerrainGPU.h"
#include "Game/Terrain/Systems/RenderTerrain.h"

#include "Game/Player/ScrollCameraSystem.h"

namespace Expanse
{
    namespace Game
    {
        class RenderWorldSystem : public SystemCollection
        {
        public:
            RenderWorldSystem(World& w, Render::IRenderer* r)
                : SystemCollection(w)
                , renderer(r)
            {}

            void Update() override
            {
                const auto window_rect = FRect{ renderer->GetWindowRect() };
                const auto view_rect = Centralized(window_rect) / world.camera_scale + world.camera_pos;
                renderer->Set2DMode(view_rect);

                SystemCollection::Update();
            }

        private:
            Render::IRenderer* renderer = nullptr;
        };


        class RenderGUISystem : public SystemCollection
        {
        public:
            RenderGUISystem(World& w, ImGuiRenderer* renderer)
                : SystemCollection(w)
                , imgui_render(renderer)
            {
            }

            void Update() override
            {
                imgui_render->StartFrame();
                SystemCollection::Update();
                imgui_render->EndFrame();
            }

        private:
            ImGuiRenderer* imgui_render;
        };
    }


    void Application::Init(Point window_size, Point framebuffer_size)
    {
        // Create renderer
        renderer = Render::CreateOpenGLRenderer(window_size, framebuffer_size);

        gui_renderer = std::make_unique<ImGuiRenderer>(renderer.get());

        SwitchToState(AppState::MainMenu);
    }

    void Application::Tick()
    {
        const auto dt = timer.Elapsed(true);

        renderer->ClearFrame();

        if (app_state)
        {
            app_state->Update(dt);

            const auto req_state = app_state->GetRequestedState();
            if (req_state != AppState::None)
            {
                SwitchToState(req_state);
            }
        }

        Input::Update();
    }

    void Application::SwitchToState(AppState new_state)
    {
        switch (new_state)
        {
        case AppState::MainMenu:
            app_state = std::make_unique<MainMenu>(renderer.get(), gui_renderer.get());
            break;
        case AppState::Game:
            app_state = std::make_unique<GameScreen>(renderer.get(), gui_renderer.get());
            break;
        case AppState::Quit:
            ready_to_quit = true;
            break;
        default:
            break;
        }
    }

    void Application::ProcessSystemEvent(const SDL_Event& evt)
    {
        ProcessInputEvent(evt);

        ImGui_ImplSDL2_ProcessEvent(&evt);
    }

    void Application::ProcessInputEvent(const SDL_Event& event)
    {
        using namespace Input;

        auto& input = g_input_state;

        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.scancode < input.keyboard_state.size()) {
                input.keyboard_state[event.key.keysym.scancode] = ButtonState::Pressed;
                input.key_state_changed = true;
            }
        }
        else if (event.type == SDL_KEYUP)
        {
            if (event.key.keysym.scancode < input.keyboard_state.size()) {
                input.keyboard_state[event.key.keysym.scancode] = ButtonState::Released;
                input.key_state_changed = true;
            }
        }
        else if (event.type == SDL_MOUSEWHEEL)
        {
            input.mouse_wheel = event.wheel.y;
        }
        else if (event.type == SDL_MOUSEMOTION)
        {
            input.mouse_pos = Point{ event.motion.x, event.motion.y };
            input.mouse_pos_rel += Point{ event.motion.xrel, -event.motion.yrel };
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            input.mouse_state[event.button.button] = ButtonState::Pressed;
            input.mouse_key_state_changed = true;
        }
        else if (event.type == SDL_MOUSEBUTTONUP)
        {
            input.mouse_state[event.button.button] = ButtonState::Released;
            input.mouse_key_state_changed = true;
        }
    }

    /************************************************************************/

    GameScreen::GameScreen(Render::IRenderer* renderer, ImGuiRenderer* gui_render)
    {
        renderer->SetBgColor({ 0.0f, 0.3f, 0.2f, 1.0f });

        InitSystems(renderer, gui_render);
    }

    void GameScreen::Update(float dt)
    {
        world.dt = dt;

        systems->Update();
    }

    void GameScreen::InitSystems(Render::IRenderer* renderer, ImGuiRenderer* gui_render)
    {
        const auto window_size = renderer->GetWindowSize();

        systems = std::make_unique<Game::SystemCollection>(world);

        systems->AddSystem<Game::Player::ScrollCamera>();

        systems->AddSystem<Game::Terrain::LoadChunks>(GetRandomSeed(), window_size);
        systems->AddSystem<Game::Terrain::UnloadChunks>(window_size);

        systems->AddSystem<Game::Terrain::LoadChunksToGPU>(renderer);
        systems->AddSystem<Game::Terrain::UnloadChunksFromGPU>(renderer);

        auto render_system = systems->AddSystem<Game::RenderWorldSystem>(renderer);
        {
            render_system->AddSystem<Game::Terrain::RenderChunks>(renderer);
        }

        auto gui_system = systems->AddSystem<Game::RenderGUISystem>(gui_render);
        {
            //gui_system->AddSystem<LogWindowSystem>();
            gui_system->AddSystem<DebugWindowSystem>();
        }
    }



    MainMenu::MainMenu(Render::IRenderer* renderer, ImGuiRenderer* gui_renderer)
        : gui_render(gui_renderer)
    {
        renderer->SetBgColor({ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    void MainMenu::Update(float dt)
    {
        gui_render->StartFrame();

        ShowMainMenu();

        gui_render->EndFrame();
    }

    void MainMenu::ShowMainMenu()
    {
        const auto MenuBtnSize = ImVec2(300, 60);

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->Size.x * 0.5f, main_viewport->Size.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        constexpr auto MenuWindowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove  | ImGuiWindowFlags_NoCollapse;
        ImGui::Begin("Main menu", nullptr, MenuWindowFlags);

        if (ImGui::Button("Play", MenuBtnSize))
        {
            SwitchTo(AppState::Game);
        }

        if (ImGui::Button("Quit", MenuBtnSize))
        {
            SwitchTo(AppState::Quit);
        }

        ImGui::End();
    }
}