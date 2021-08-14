#pragma once

#include "Utils/Array2D.h"

namespace Expanse::Game::Terrain
{
	enum class TerrainType
	{
		Dirt,
		Grass
	};

	struct TerrainCell
	{
		TerrainType type;
		int height;
	};

	struct TerrainChunk
	{
		static constexpr int Size = 16;

		Point position;
		Array2D<TerrainCell> cells;

		explicit TerrainChunk(Point pos)
			: position(pos)
			, cells(Rect{0, 0, Size, Size})
		{}
	};
}