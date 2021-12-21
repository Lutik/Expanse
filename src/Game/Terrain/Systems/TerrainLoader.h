#pragma once

#include "Game/ISystem.h"
#include "Game/Terrain/Components/TerrainData.h"

namespace Expanse::Game::Terrain
{
	class ITerrainLoader
	{
	public:
		virtual ~ITerrainLoader() = default;
		virtual bool LoadChunk(TerrainChunk& chunk) = 0;
	};
}