#pragma once

#include "Render/IRenderer.h"
#include "Render/VertexTypes.h"

namespace Expanse
{
    struct GameObject
    {
        GameObject(Render::IRenderer* renderer, const std::string& mat_file, const std::vector<Render::VertexP2T2>& verts);

        Render::Material material;
        Render::Mesh mesh;
        FPoint position;
    };

    class Application
    {
    public:
        void Init();

        void Tick();

    private:
        std::unique_ptr<Render::IRenderer> renderer;

        std::vector<GameObject> objects;
    };
}