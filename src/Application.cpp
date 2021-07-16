#include "Application.h"

namespace Expanse
{
    void Application::Init()
    {
        renderer = Render::CreateOpenGLRenderer();

        material = renderer->CreateMaterial("content/shaders/basic.txt");
        mesh = renderer->CreateMesh();

        const std::vector<Render::VertexP2> verts = {
            {{-0.8f, -0.9f}}, {{0.8f, -0.9f}}, {{0.0f, 0.9f}}
        };
        renderer->SetMeshVertices(mesh, verts);
    }

    void Application::Tick()
    {
        renderer->ClearFrame();
        renderer->Draw(mesh, material);
    }
}