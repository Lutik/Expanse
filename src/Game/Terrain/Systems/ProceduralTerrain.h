#pragma once

#include "Game/Terrain/Systems/TerrainLoader.h"
#include "Utils/PerlinNoiseGenerator.h"

namespace Expanse::Game::Terrain
{
	class TerrainLoader_Procedural : public ITerrainLoader
	{
	public:
		TerrainLoader_Procedural(uint32_t seed);

		bool HasChunk(Point pos) const override;

		std::future<TerrainCellsArray> LoadChunk(Point pos) override;

	private:
		uint32_t type_seeds[3];

		TerrainType GetTerrainAt(Point cell_pos) const;

		PerlinNoiseGenerator height_gen;

		TerrainCellsArray LoadChunk_Internal(Point pos);
	};
}