#include "pch.h"

#include "ProceduralTerrain.h"

#include "Game/CoordSystems.h"

#include "Utils/Random.h"
#include "Utils/RectPoints.h"
#include "Utils/Utils.h"

namespace Expanse::Game::Terrain
{
	static const std::vector<NoiseHarmonics> height_harmonics = {
		{ 0.05f, -5.0f, 5.0f },
		{ 0.27f, -3.0f, 3.0f },
	};

	TerrainLoader_Procedural::TerrainLoader_Procedural(uint32_t seed)
		: height_gen(Squirrel3(100, seed), height_harmonics)
	{
		const auto types_init_seed = Squirrel3(0, seed);
		GenerateSeeds(type_seeds, types_init_seed);
	}

	bool TerrainLoader_Procedural::HasChunk(Point pos) const
	{
		return true;
	}

	std::future<Array2D<TerrainCell>> TerrainLoader_Procedural::LoadChunk(Point chunk_pos)
	{
		return std::async(std::launch::async, [this, chunk_pos](){ return LoadChunk_Internal(chunk_pos); });
	}

	Array2D<TerrainCell> TerrainLoader_Procedural::LoadChunk_Internal(Point chunk_pos)
	{
		Array2D<TerrainCell> cells{ TerrainChunk::Area };

		for (const auto local_pos : utils::rect_points{ cells.GetRect() })
		{
			const auto cell_pos = Coords::LocalToCell(local_pos, chunk_pos, TerrainChunk::Size);

			auto& cell = cells[local_pos];

			cell.type = GetTerrainAt(cell_pos);
			cell.height = static_cast<int>(height_gen.Get(FPoint{ cell_pos }));
		}

		return cells;
	}

	TerrainType TerrainLoader_Procedural::GetTerrainAt(Point cell_pos) const
	{
		const auto pt = FPoint{ cell_pos } * 0.07f;

		const std::array<float, 3> values = {
			PerlinNoise(pt, type_seeds[0]),
			PerlinNoise(pt, type_seeds[1]),
			PerlinNoise(pt, type_seeds[2]) * 0.7f,
		};

		const auto index = std::distance(std::ranges::begin(values), std::ranges::max_element(values));

		return static_cast<TerrainType>(index);
	}
}