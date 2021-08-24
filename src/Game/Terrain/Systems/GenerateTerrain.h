#pragma once

#include "Game/ISystem.h"
#include "Game/Terrain/TerrainData.h"

#include "Utils/Math.h"

namespace Expanse::Game::Terrain
{
	class GenerateChunks : public ISystem
	{
	public:
		GenerateChunks(World& w, uint32_t seed, Point window_size);

		void Update() override;

	private:
		Point window_size;
		uint32_t types_seed;
		uint32_t heights_seed[2];
		bool init = false;

		float GetHeightAt(Point cell_pos) const;
		TerrainType GetTerrainAt(Point cell_pos) const;

		void LoadChunk(TerrainChunk& chunk);
	};

	class UnloadChunks : public ISystem
	{
	public:
		UnloadChunks(World& w, Point window_size);

		void Update() override;

	private:
		Point window_size;
	};
}