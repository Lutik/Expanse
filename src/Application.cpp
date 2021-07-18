#include "Application.h"

#include "Image/Image.h"

namespace Expanse
{
    void Application::Init()
    {
        renderer = Render::CreateOpenGLRenderer();

        material[0] = renderer->CreateMaterial("content/materials/basic_textured.json");
        material[1] = renderer->CreateMaterial(material[0]); // duplicate material to use other set of parameters

        auto tex1 = renderer->CreateTexture("content/textures/wood.png");
        renderer->SetMaterialParameter(material[0], "tex0", tex1);

        auto tex2 = renderer->CreateTexture("content/textures/concrete.png");
        renderer->SetMaterialParameter(material[1], "tex0", tex2);


        const std::vector<Render::VertexP2T2> verts1 = {
            {{-0.8f, -0.9f}, {0.0f, 0.0f}},
            {{0.8f, -0.9f}, {1.0f, 0.0f}},
            {{0.0f, 0.9f}, {0.5f, 1.0f}},
        };
        const std::vector<Render::VertexP2T2> verts2 = {
            {{-0.8f, -0.9f}, {0.0f, 0.0f}},
            {{0.0f, 0.9f}, {0.5f, 1.0f}},
            {{-0.8, 0.9f}, {0.0f, 1.0f}},
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