#pragma once

#include "Render/IRenderer.h"
#include "Render/VertexTypes.h"

namespace Expanse
{
    class Application
    {
    public:
        void Init();

        void Tick();

    private:
        std::unique_ptr<Render::IRenderer> renderer;

        Render::Material material[2];
        Render::Mesh mesh[2];
    };
}