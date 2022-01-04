#include "pch.h"

#include "ProceduralTerrain.h"

#include "Game/CoordSystems.h"

#include "Utils/Random.h"
#include "Utils/RectPoints.h"
#include "Utils/Utils.h"

namespace Expanse::Game::Terrain
{
	struct NoiseParam
	{
		float freq;
		float min;
		float max;
	};

	TerrainLoader_Procedural::TerrainLoader_Procedural(uint32_t seed)
	{
		const auto types_init_seed = Squirrel3(0, seed);
		GenerateSeeds(type_seeds, types_init_seed);

		const auto heights_init_seed = Squirrel3(100, seed);
		GenerateSeeds(height_seeds, heights_init_seed);
	}

	bool TerrainLoader_Procedural::LoadChunk(TerrainChunk& chunk)
	{
		for (const auto local_pos : utils::rect_points{ chunk.cells.GetRect() })
		{
			const auto cell_pos = Coords::LocalToCell(local_pos, chunk.position, TerrainChunk::Size);

			auto& cell = chunk.cells[local_pos];

			cell.type = GetTerrainAt(cell_pos);
			cell.height = static_cast<int>(GetHeightAt(cell_pos));
		}

		return true;
	}

	float TerrainLoader_Procedural::GetHeightAt(Point cell_pos) const
	{
		static constexpr NoiseParam heights_noise_params[] = {
			{ 0.05f, -5.0f, 5.0f },
			{ 0.27f, -3.0f, 3.0f },
		};

		float height = 0.0f;
		utils::for_each_zipped(height_seeds, heights_noise_params, [&height, cell_pos](uint32_t seed, const auto& params)
		{
			const auto pt = FPoint{ cell_pos } * params.freq;
			height += Lerp(params.min, params.max, PerlinNoise(pt, seed));
		});

		return height;
	}

	TerrainType TerrainLoader_Procedural::GetTerrainAt(Point cell_pos) const
	{
		const auto pt = FPoint{ cell_pos } * 0.07f;

		const std::array<float, 3> values = {
			PerlinNoise(pt, type_seeds[0]),
			PerlinNoise(pt, type_seeds[1]),
			PerlinNoise(pt, type_seeds[2]),
		};

		const auto index = std::distance(std::ranges::begin(values), std::ranges::max_element(values));

		return static_cast<TerrainType>(index);
	}
}