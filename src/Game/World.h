#pragma once

#include "Render/RenderTypes.h"
#include "Utils/Math.h"

#include "Input/Input.h"

#include <vector>

#include "ECS/World.h"

namespace Expanse::Game
{
    struct World
    {
        ecs::World entities;

        Input::InputState input;
        float dt;
    };
}