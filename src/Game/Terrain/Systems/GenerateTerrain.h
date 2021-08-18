#pragma once

#include "Game/ISystem.h"
#include "Game/Terrain/TerrainData.h"

#include "Utils/Math.h"

namespace Expanse::Game::Terrain
{
	class GenerateCells : public ISystem
	{
	public:
		GenerateCells(World& w, uint32_t seed);

		void Update();

	private:
		uint32_t types_seed;
		uint32_t heights_seed[2];
		bool init = false;

		float GetHeightAt(Point cell_pos) const;
		TerrainType GetTerrainAt(Point cell_pos) const;
	};
}