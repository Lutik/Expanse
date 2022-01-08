#pragma once

#include "Game/Terrain/Systems/TerrainLoader.h"
#include "Utils/PerlinNoiseGenerator.h"

namespace Expanse::Game::Terrain
{
	class TerrainLoader_Procedural : public ITerrainLoader
	{
	public:
		TerrainLoader_Procedural(uint32_t seed);

		bool LoadChunk(TerrainChunk& chunk);

	private:
		uint32_t type_seeds[3];

		TerrainType GetTerrainAt(Point cell_pos) const;

		PerlinNoiseGenerator height_gen;
	};
}