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
		uint32_t type_seeds[3];
		uint32_t height_seeds[2];

		float GetHeightAt(Point cell_pos) const;
		TerrainType GetTerrainAt(Point cell_pos) const;
	};
}