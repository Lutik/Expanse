#pragma once

#include "Render/IRenderer.h"
#include "Render/VertexTypes.h"

namespace Expanse
{
    struct GameObject
    {    
        Render::Mesh mesh;
        Render::Material material;
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