#include "Application.h"

#include "Image/Image.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace Expanse
{
    void Application::Init()
    {
        const std::vector<Render::VertexP2T2> verts = {
            {{-50.0f, -50.0f}, {0.0f, 0.0f}},
            {{-50.0f, 50.0f}, {0.0f, 1.0f}},
            {{50.0f, 50.0f}, {1.0f, 1.0f}},
            {{50.0f, -50.0f}, {1.0f, 0.0f}},
        };
        const std::vector<uint16_t> indices = { 1, 0, 2, 3 };

        renderer = Render::CreateOpenGLRenderer();

        auto mesh = renderer->CreateMesh(verts, indices, Render::PrimitiveType::TriangleStrip);
        auto mat0 = renderer->CreateMaterial("content/materials/wood.json");
        auto mat1 = renderer->CreateMaterial("content/materials/concrete.json");

        objects = {
            { mesh, mat0, { 400.0f, 200.0f }, 100.0f },
            { mesh, mat1, { 1000.0f, 600.0f}, 150.0f },
        };
    }

    void Application::Tick()
    {
        // uodate input
        Input::UpdateState(input);

        // calculate dt
        const float dt = timer.Elapsed(true);

        // move objects
        FPoint offset{ 0.0f, 0.0f };
        if (input.IsKeyDown(Input::Key::Up)) offset += { 0.0f, 1.0f };
        if (input.IsKeyDown(Input::Key::Down)) offset += { 0.0f, -1.0f };
        if (input.IsKeyDown(Input::Key::Left)) offset += { -1.0f, 0.0f };
        if (input.IsKeyDown(Input::Key::Right)) offset += { 1.0f, 0.0f };
        for (auto& obj : objects) {
            obj.position += offset * (obj.speed * dt);
        }

        // render objects
        renderer->ClearFrame();
        renderer->Set2DMode(1440, 800);
        for (auto& obj : objects)
        {
            auto model = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ obj.position.x, obj.position.y, 0.0f });
            renderer->SetMaterialParameter(obj.material, "model", model);
            renderer->Draw(obj.mesh, obj.material);
        }
    }

    void Application::ProcessSystemEvent(const SDL_Event& evt)
    {
        Input::ProcessEvent(evt, input);
    }
}