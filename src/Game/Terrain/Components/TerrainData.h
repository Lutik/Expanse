#pragma once

#include "Utils/Array2D.h"
#include "ECS/Entity.h"

namespace Expanse::Game::Terrain
{
	enum class TerrainType
	{
		Dirt,
		Grass
	};

	struct TerrainCell
	{
		TerrainType type = TerrainType::Dirt;
		int height = 0;
	};

	struct TerrainChunk
	{
		static constexpr int Size = 32;
		static constexpr Rect Area = {0, 0, Size, Size};


		Point position;
		Array2D<TerrainCell> cells;
		int use_count = 0;


		explicit TerrainChunk(Point pos)
			: position(pos)
			, cells(Area)
		{}
	};


	namespace Event
	{
		struct ChunkLoaded {};
	}

	struct ChunkMap
	{
		Array2D<ecs::Entity> chunks;
	};
}