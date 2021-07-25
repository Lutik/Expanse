#pragma once

#include "Render/IRenderer.h"
#include "Render/VertexTypes.h"

#include "Utils/Timers.h"

#include "Input/Input.h"

#include <SDL.h>

namespace Expanse
{
    struct GameObject
    {    
        Render::Mesh mesh;
        Render::Material material;
        FPoint position;
     
        float speed = 1.0f;
    };

    class Application
    {
    public:
        void Init();

        void Tick();

        void ProcessSystemEvent(const SDL_Event& evt);
    private:
        std::unique_ptr<Render::IRenderer> renderer;

        std::vector<GameObject> objects;

        Timer timer;

        Input::InputState input;
    };
}