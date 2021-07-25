#pragma once

#include "Render/RenderTypes.h"
#include "Utils/Math.h"

#include "Input/Input.h"

#include <vector>

namespace Expanse::Game
{
    struct GameObject
    {
        Render::Mesh mesh;
        Render::Material material;
        FPoint position;

        float speed = 1.0f;
    };


    struct World
    {
        std::vector<GameObject> objects;

        Input::InputState input;
        float dt;
    };
}