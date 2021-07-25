#include "Application.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_opengl3.h"

namespace Expanse
{
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

            for (auto& obj : world.objects) {
                obj.position += offset * (obj.speed * world.dt);
            }
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
                for (auto& obj : world.objects)
                {
                    auto model = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ obj.position.x, obj.position.y, 0.0f });
                    renderer->SetMaterialParameter(obj.material, "model", model);
                    renderer->Draw(obj.mesh, obj.material);
                }
            }
        }

    private:
        Render::IRenderer* renderer = nullptr;
    };

    Application::Application()
    {
        systems = std::make_unique<Game::SystemCollection>(world);
    }

    void Application::Init(Point window_size, Point framebuffer_size)
    {
        // Create renderer
        renderer = Render::CreateOpenGLRenderer(window_size, framebuffer_size);

        // Init game objects
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

        world.objects = {
            { mesh, mat0, { 400.0f, 200.0f }, 100.0f },
            { mesh, mat1, { 1000.0f, 600.0f}, 150.0f },
        };

        // Init systems
        auto logic_systems = systems->AddSystem<Game::SystemCollection>();
        logic_systems->AddSystem<MoveObjectsSystem>();

        auto render_systems = systems->AddSystem<Game::SystemCollection>();
        render_systems->AddSystem<RenderObjectsSystem>(renderer.get());
    }

    void Application::Tick()
    {
        world.dt = timer.Elapsed(true);

        systems->Update();

        ImGuiFrame();

        Input::UpdateState(world.input);
    }

    void Application::ProcessSystemEvent(const SDL_Event& evt)
    {
        Input::ProcessEvent(evt, world.input);

        ImGui_ImplSDL2_ProcessEvent(&evt);
    }

    void Application::ImGuiFrame()
    {
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();

        //glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        //glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}