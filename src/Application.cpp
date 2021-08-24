#include "Application.h"

#include "backends/imgui_impl_sdl.h"
#include "GUI/ImGuiRenderer.h"
#include "GUI/LogWindow.h"

#include "Render/SpriteBatch.h"
#include "Utils/Random.h"

#include "Game/Terrain/Systems/GenerateTerrain.h"
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
            RenderGUISystem(World& w, Render::IRenderer* renderer)
                : SystemCollection(w)
            {
                imgui_render = std::make_unique<ImGuiRenderer>(renderer);
            }

            void Update() override
            {
                ImGui_ImplSDL2_NewFrame();

                imgui_render->StartFrame();

                SystemCollection::Update();
                //ImGui::ShowDemoWindow();

                imgui_render->EndFrame();
            }

        private:
            std::unique_ptr<ImGuiRenderer> imgui_render;
        };
    }


    Application::Application()
    {
        systems = std::make_unique<Game::SystemCollection>(world);
    }

    void Application::Init(Point wnd_size, Point framebuffer_size)
    {
        window_size = wnd_size;

        // Create renderer
        renderer = Render::CreateOpenGLRenderer(window_size, framebuffer_size);
        renderer->SetBgColor({0.0f, 0.3f, 0.2f, 1.0f});

        InitSystems();
    }

    void Application::InitSystems()
    {
        auto render = renderer.get();

        systems->AddSystem<Game::Player::ScrollCamera>();

        systems->AddSystem<Game::Terrain::GenerateChunks>(GetRandomSeed(), window_size);
        systems->AddSystem<Game::Terrain::UnloadChunks>(window_size);

        systems->AddSystem<Game::Terrain::LoadChunksToGPU>(render);
        systems->AddSystem<Game::Terrain::UnloadChunksFromGPU>(render);

        auto render_system = systems->AddSystem<Game::RenderWorldSystem>(render);
        {
            render_system->AddSystem<Game::Terrain::RenderChunks>(render);
        }

        auto gui_system = systems->AddSystem<Game::RenderGUISystem>(render);
        {
            gui_system->AddSystem<LogWindowSystem>();
        }
    }

    void Application::Tick()
    {
        world.dt = timer.Elapsed(true);

        renderer->ClearFrame();

        systems->Update();

        Input::UpdateState(world.input);
    }

    void Application::ProcessSystemEvent(const SDL_Event& evt)
    {
        ProcessInputEvent(evt);

        ImGui_ImplSDL2_ProcessEvent(&evt);
    }

    void Application::ProcessInputEvent(const SDL_Event& event)
    {
        using namespace Input;

        auto& input = world.input;

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
}