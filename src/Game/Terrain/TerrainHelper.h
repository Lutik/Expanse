#pragma once

#include "TerrainData.h"

#include "ECS/Entity.h"
#include "Utils/Array2D.h"

#include <unordered_map>

namespace Expanse::Game
{
	struct World;
}

namespace Expanse::Game::Terrain
{
	class TerrainHelper
	{
	public:
		TerrainHelper(World& w) : world(w) {}

		void Update();

		TerrainCell* GetCell(Point cell_pos) const;
	private:
		World& world;
		std::unordered_map<Point, ecs::Entity> chunks;
	};
}