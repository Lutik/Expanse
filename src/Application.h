#pragma once

#include "Render/IRenderer.h"
#include "Render/VertexTypes.h"

#include "Utils/Timers.h"

namespace Expanse
{
    struct GameObject
    {    
        Render::Mesh mesh;
        Render::Material material;
        FPoint position;
     
        float radius = 100.0;
        float speed = 1.0f;
        float angle = 0.0f;
    };

    class Application
    {
    public:
        void Init();

        void Tick();

    private:
        std::unique_ptr<Render::IRenderer> renderer;

        std::vector<GameObject> objects;

        Timer timer;
    };
}