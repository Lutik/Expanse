#include "Application.h"

#include "Image/Image.h"

namespace Expanse
{
    void Application::Init()
    {
        renderer = Render::CreateOpenGLRenderer();

        material[0] = renderer->CreateMaterial("content/materials/wood.json");
        material[1] = renderer->CreateMaterial("content/materials/concrete.json");        

        const std::vector<Render::VertexP2T2> verts1 = {
            {{-0.8f, -0.9f}, {0.0f, 0.0f}},
            {{0.8f, -0.9f}, {1.0f, 0.0f}},
            {{0.0f, 0.9f}, {0.5f, 1.0f}},
        };
        const std::vector<Render::VertexP2T2> verts2 = {
            {{-0.8f, -0.9f}, {0.0f, 0.0f}},
            {{0.0f, 0.9f}, {0.5f, 1.0f}},
            {{-0.8f, 0.9f}, {0.0f, 1.0f}},
        };
        mesh[0] = renderer->CreateMesh(verts1);
        mesh[1] = renderer->CreateMesh(verts2);
    }

    void Application::Tick()
    {
        renderer->ClearFrame();

        renderer->Draw(mesh[0], material[0]);
        renderer->Draw(mesh[1], material[1]);
    }
}