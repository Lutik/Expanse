#include "Application.h"

namespace Expanse
{
    void Application::Init()
    {
        renderer = Render::CreateOpenGLRenderer();
        shader = { "content/shaders/basic.txt" };

        const std::vector<Render::VertexP2> verts = {
            {{-1.0f, -1.0f}}, {{1.0f, -1.0f}}, {{0.0f, 1.0f}}
        };
        vao.SetVertices(verts);
    }

    void Application::Tick()
    {
        renderer->ClearFrame();

        shader.Bind();
        vao.Draw();
    }
}