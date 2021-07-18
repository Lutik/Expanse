#include "Application.h"

#include "Image/Image.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace Expanse
{
    GameObject::GameObject(Render::IRenderer* renderer, const std::string& mat_file, const std::vector<Render::VertexP2T2>& verts)
    {
        material = renderer->CreateMaterial(mat_file);
        mesh = renderer->CreateMesh(verts);
    }

    void Application::Init()
    {
        const std::vector<Render::VertexP2T2> verts = {
            {{-100.0f, -60.0f}, {0.0f, 0.0f}},
            {{100.0f, -60.0f}, {1.0f, 0.0f}},
            {{0.0f, 120.0f}, {0.5f, 1.0f}},
        };

        renderer = Render::CreateOpenGLRenderer();

        objects.emplace_back(renderer.get(), "content/materials/wood.json", verts);
        objects.emplace_back(renderer.get(), "content/materials/concrete.json", verts);

        auto proj = glm::orthoLH_NO(0.0f, 1440.0f, 0.0f, 800.0f, -1.0f, 1.0f);
        renderer->SetMaterialParameter(objects[0].material, "proj", proj);
        renderer->SetMaterialParameter(objects[1].material, "proj", proj);

        objects[0].position = { 400.0f, 200.0f };
        objects[1].position = { 1000.0f, 600.0f };
    }

    void Application::Tick()
    {
        renderer->ClearFrame();

        for (const auto& obj : objects)
        {
            auto model = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ obj.position.x, obj.position.y, 0.0f });
            renderer->SetMaterialParameter(obj.material, "model", model);
            renderer->Draw(obj.mesh, obj.material);
        }
    }
}