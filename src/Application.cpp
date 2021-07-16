#include "Application.h"

namespace Expanse
{
    void Application::Init()
    {
        renderer = Render::CreateOpenGLRenderer();

        material = renderer->CreateMaterial("content/materials/basic.json");

        const std::vector<Render::VertexP2> verts = {
            {{-0.8f, -0.9f}}, {{0.8f, -0.9f}}, {{0.0f, 0.9f}}
        };
        mesh = renderer->CreateMesh(verts);
    }

    void Application::Tick()
    {
        renderer->ClearFrame();
        renderer->Draw(mesh, material);
    }
}