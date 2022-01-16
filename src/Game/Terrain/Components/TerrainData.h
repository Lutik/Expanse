#pragma once

#include "Utils/Array2D.h"
#include "ECS/Entity.h"

#include <future>

namespace Expanse::Game::Terrain
{
	using TerrainType = int;

	struct TerrainCellsArray
	{
		Array2D<TerrainType> types;
		Array2D<float> heights;

		TerrainCellsArray() = default;

		explicit TerrainCellsArray(const Rect& area)
			: types(area, 0)
			, heights({area.x, area.y, area.w + 1, area.h + 1}, 0.0f)
		{}
	};

	struct TerrainChunk
	{
		static constexpr int Size = 32;
		static constexpr Rect Area = {0, 0, Size, Size};
		static constexpr Rect AreaVtx = {0, 0, Size+1, Size+1};

		Point position;
		int use_count = 0;
		TerrainCellsArray cells;

		TerrainChunk() = default;

		explicit TerrainChunk(Point pos)
			: position(pos)
			, cells(Area)
		{}
	};

	struct AsyncLoadingChunk
	{
		Point position;
		std::future<TerrainCellsArray> data;
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