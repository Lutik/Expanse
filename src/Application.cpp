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
        const std::vector<uint16_t> indices = { 0, 2, 1, 0, 3, 2 };

        renderer = Render::CreateOpenGLRenderer();

        auto mesh = renderer->CreateMesh(verts, indices);
        auto mat0 = renderer->CreateMaterial("content/materials/wood.json");
        auto mat1 = renderer->CreateMaterial("content/materials/concrete.json");

        objects = {
            { mesh, mat0, { 400.0f, 200.0f }},
            { mesh, mat1, { 1000.0f, 600.0f}},
        };
    }

    void Application::Tick()
    {
        renderer->ClearFrame();

        renderer->Set2DMode(1440, 800);
        for (const auto& obj : objects)
        {
            auto model = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ obj.position.x, obj.position.y, 0.0f });
            renderer->SetMaterialParameter(obj.material, "model", model);
            renderer->Draw(obj.mesh, obj.material);
        }
    }
}