#include "Application.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "backends/imgui_impl_sdl.h"

namespace Expanse
{
    /* Components */

    struct Visual
    {
        Render::Mesh mesh;
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
        {}

        void Update() override
        {
            if (renderer)
            {
                renderer->ClearFrame();
                renderer->Set2DMode();

                world.entities.ForEach<Visual, Position>([this](auto ent, const Visual& visual, const Position& pos)
                {
                    auto model = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ pos.position.x, pos.position.y, 0.0f });
                    renderer->SetMaterialParameter(visual.material, "model", model);
                    renderer->Draw(visual.mesh, visual.material);
                });
            }
        }

    private:
        Render::IRenderer* renderer = nullptr;
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
        const std::vector<Render::VertexP2T2> verts = {
            {{-50.0f, -50.0f}, {0.0f, 0.0f}},
            {{-50.0f, 50.0f}, {0.0f, 1.0f}},
            {{50.0f, 50.0f}, {1.0f, 1.0f}},
            {{50.0f, -50.0f}, {1.0f, 0.0f}},
        };
        const std::vector<uint16_t> indices = { 1, 0, 2, 3 };

        auto mesh = renderer->CreateMesh(verts, indices, Render::PrimitiveType::TriangleStrip);
        auto mat0 = renderer->CreateMaterial("content/materials/wood.json");
        auto mat1 = renderer->CreateMaterial("content/materials/concrete.json");

        // Create game entities
        auto& entities = world.entities;

        auto ent1 = entities.CreateEntity();
        entities.AddComponent<Visual>(ent1, mesh, mat0);
        entities.AddComponent<Position>(ent1, FPoint{ 400.0f, 300.0f });
        entities.AddComponent<Speed>(ent1, 100.0f);

        auto ent2 = entities.CreateEntity();
        entities.AddComponent<Visual>(ent2, mesh, mat1);
        entities.AddComponent<Position>(ent2, FPoint{ 1000.0f, 600.0f });
        entities.AddComponent<Speed>(ent2, 150.0f);

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

        ImGuiFrame(world.dt);

        Input::UpdateState(world.input);
    }

    void Application::ProcessSystemEvent(const SDL_Event& evt)
    {
        Input::ProcessEvent(evt, world.input);

        ImGui_ImplSDL2_ProcessEvent(&evt);
    }

    void Application::ImGuiFrame(float dt)
    {
        ImGui_ImplSDL2_NewFrame(renderer->GetWindowSize(), renderer->GetFramebufferSize(), dt);

        imgui_render->StartFrame();

        ImGui::ShowDemoWindow();

        imgui_render->EndFrame();
    }
}