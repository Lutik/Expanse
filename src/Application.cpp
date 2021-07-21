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
            { mesh, mat0, { 400.0f, 200.0f }, 80.0f, 1.0f, 0.0f },
            { mesh, mat1, { 1000.0f, 600.0f}, 120.0f, -1.0f, 0.0f },
        };
    }

    void Application::Tick()
    {
        const float dt = timer.Elapsed(true);

        renderer->ClearFrame();

        renderer->Set2DMode(1440, 800);
        for (auto& obj : objects)
        {
            obj.angle += obj.speed * dt;

            const FPoint pos = obj.position + FPoint{ std::sin(obj.angle), std::cos(obj.angle) } * obj.radius;

            auto model = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ pos.x, pos.y, 0.0f });
            renderer->SetMaterialParameter(obj.material, "model", model);
            renderer->Draw(obj.mesh, obj.material);
        }
    }
}