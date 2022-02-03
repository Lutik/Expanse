#include "pch.h"

#include "ProceduralTerrain.h"

#include "Game/CoordSystems.h"

#include "Utils/Random.h"
#include "Utils/RectPoints.h"
#include "Utils/Utils.h"
#include "Utils/Async.h"

namespace Expanse::Game::Terrain
{
	static const std::vector<NoiseHarmonics> height_harmonics = {
		{ 0.05f, -6.0f, 6.0f },
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

	std::future<TerrainCellsArray> TerrainLoader_Procedural::LoadChunk(Point chunk_pos)
	{
		return utils::Async(&TerrainLoader_Procedural::LoadChunk_Internal, this, chunk_pos);
	}

	TerrainCellsArray TerrainLoader_Procedural::LoadChunk_Internal(Point chunk_pos)
	{
		TerrainCellsArray cells{ TerrainChunk::Area };

		// load terrain types
		for (const auto local_pos : utils::rect_points{ cells.types.GetRect() })
		{
			const auto cell_pos = Coords::LocalToCell(local_pos, chunk_pos, TerrainChunk::Size);
			cells.types[local_pos] = GetTerrainAt(cell_pos);
		}

		// load heightmap
		for (const auto local_pos : utils::rect_points{ cells.heights.GetRect() })
		{
			const auto cell_pos = Coords::LocalToCell(local_pos, chunk_pos, TerrainChunk::Size);
			cells.heights[local_pos] = static_cast<HeightType>(height_gen.Get(FPoint{ cell_pos }));
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