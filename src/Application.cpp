#include "Application.h"

#include "backends/imgui_impl_sdl.h"

#include "Render/SpriteBatch.h"
#include "Utils/Random.h"

namespace Expanse
{
    /* Components */

    struct Visual
    {
        Render::Material material;
    };

    struct Position
    {
        FPoint position;
    };

    struct Speed
    {
        float speed;
    };

    /* Systems */

    class MoveObjectsSystem final : public Game::ISystem
    {
    public:
        MoveObjectsSystem(Game::World& w) : ISystem(w) {}

        void Update() override
        {
            FPoint offset{ 0.0f, 0.0f };
            if (world.input.IsKeyDown(Input::Key::Up)) offset += { 0.0f, 1.0f };
            if (world.input.IsKeyDown(Input::Key::Down)) offset += { 0.0f, -1.0f };
            if (world.input.IsKeyDown(Input::Key::Left)) offset += { -1.0f, 0.0f };
            if (world.input.IsKeyDown(Input::Key::Right)) offset += { 1.0f, 0.0f };

            world.entities.ForEach<Position, Speed>([offset, dt = world.dt](ecs::Entity ent, Position& pos, const Speed& sp)
            {
                pos.position += offset * (sp.speed * dt);
            });
        }
    };

    class RenderObjectsSystem final : public Game::ISystem
    {
    public:
        RenderObjectsSystem(Game::World& w, Render::IRenderer *r)
            : ISystem(w)
            , renderer(r)
            , batch(r)
        {
        }

        void Update() override
        {
            static constexpr std::array<Render::VertexP2T2, 4> verts = {{
                {{-50.0f, -50.0f}, {0.0f, 0.0f}},
                {{-50.0f, 50.0f}, {0.0f, 1.0f}},
                {{50.0f, 50.0f}, {1.0f, 1.0f}},
                {{50.0f, -50.0f}, {1.0f, 0.0f}},
            }};
            static constexpr std::array<uint16_t, 6> indices = { 1, 0, 2, 2, 0, 3 };

            if (renderer)
            {
                renderer->ClearFrame();
                renderer->Set2DMode();

                world.entities.ForEach<Visual, Position>([this](auto ent, const Visual& visual, const Position& pos)
                {
                    batch.DrawWithOffset(pos.position, verts, indices, visual.material);
                });
            }


            batch.Draw(verts, indices, Render::Material{});
        }

    private:
        Render::IRenderer* renderer = nullptr;
        Render::SpriteBatch<Render::VertexP2T2> batch;
    };

    /********************************************************************/

    Application::Application()
    {
        systems = std::make_unique<Game::SystemCollection>(world);
    }

    void Application::Init(Point window_size, Point framebuffer_size)
    {
        // Create renderer
        renderer = Render::CreateOpenGLRenderer(window_size, framebuffer_size);
        renderer->SetBgColor({0.0f, 0.6f, 0.4f, 1.0f});

        // Init graphical objects
        Render::Material mat[2];
        mat[0] = renderer->CreateMaterial("content/materials/wood.json");
        mat[1] = renderer->CreateMaterial("content/materials/concrete.json");

        // Create game entities
        auto& entities = world.entities;
        for (int i : std::views::iota(0, 20))
        {
            auto ent = entities.CreateEntity();
            entities.AddComponent<Visual>(ent, mat[RandomInt(0,1)]);
            entities.AddComponent<Position>(ent, FPoint{ RandomFloat(0.0f, 1000.0f), RandomFloat(0.0f, 800.0f) });
            entities.AddComponent<Speed>(ent, RandomFloat(50.0f, 150.0f));
        }


        // Init systems
        auto logic_systems = systems->AddSystem<Game::SystemCollection>();
        logic_systems->AddSystem<MoveObjectsSystem>();

        auto render_systems = systems->AddSystem<Game::SystemCollection>();
        render_systems->AddSystem<RenderObjectsSystem>(renderer.get());

        imgui_render = std::make_unique<ImGuiRenderer>(renderer.get());
    }

    void Application::Tick()
    {
        world.dt = timer.Elapsed(true);

        systems->Update();

        //ImGuiFrame(world.dt);

        Input::UpdateState(world.input);
    }

    void Application::ProcessSystemEvent(const SDL_Event& evt)
    {
        ProcessInputEvent(evt);

        ImGui_ImplSDL2_ProcessEvent(&evt);
    }

    void Application::ImGuiFrame(float dt)
    {
        ImGui_ImplSDL2_NewFrame(renderer->GetWindowSize(), renderer->GetFramebufferSize(), dt);

        imgui_render->StartFrame();

        ImGui::ShowDemoWindow();

        imgui_render->EndFrame();
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