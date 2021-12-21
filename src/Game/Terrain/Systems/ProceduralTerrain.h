#pragma once

#include "Game/Terrain/Systems/TerrainLoader.h"

namespace Expanse::Game::Terrain
{
	class TerrainLoader_Procedural : public ITerrainLoader
	{
	public:
		TerrainLoader_Procedural(uint32_t seed);

		bool LoadChunk(TerrainChunk& chunk);

	private:
		uint32_t types_seed;
		uint32_t heights_seed[2];

		float GetHeightAt(Point cell_pos) const;
		TerrainType GetTerrainAt(Point cell_pos) const;
	};
}