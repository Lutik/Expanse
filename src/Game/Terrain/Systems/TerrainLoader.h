#pragma once

#include "Game/ISystem.h"
#include "Game/Terrain/Components/TerrainData.h"

namespace Expanse::Game::Terrain
{
	class ITerrainLoader
	{
	public:
		virtual ~ITerrainLoader() = default;

		virtual bool HasChunk(Point chunk) const = 0;

		virtual std::future<Array2D<TerrainCell>> LoadChunk(Point pos) = 0;
	};
}