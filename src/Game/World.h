#pragma once

#include "Render/RenderTypes.h"
#include "Utils/Math.h"

#include <vector>

#include "ECS/World.h"

namespace Expanse::Game
{
    struct World
    {
        // ECS entities and components
        ecs::World entities;
        ecs::Globals globals;

        // Coordinate systems data
        Point world_origin = { 0, 0 }; // in cell space
        FPoint camera_pos = { 0.0f, 0.0f }; // in scene space
        float camera_scale = 64.0f; // window pixels per scene unit

        // Frame delta time
        float dt = 0.0f;
    };
}