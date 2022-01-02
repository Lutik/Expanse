#include "TerrainHelpers.h"

#include "Components/TerrainData.h"
#include "Utils/Bounds.h"
#include "Utils/RectPoints.h"

#include <ranges>

namespace Expanse::Game::Terrain
{
	void UpdateChunkMap(World& world)
	{
		auto* map = world.globals.GetOrCreate<ChunkMap>();
		
		// Calculate bounding rect of all chunks
		const auto& loaded_chunks = world.entities.GetComponentArray<TerrainChunk>();
		auto get_pos = [](const TerrainChunk& chunk) { return chunk.position; };
		const auto bounds = utils::CalcBounds(loaded_chunks | std::views::transform(get_pos));

		// Write entities into map
		if (bounds.w > 0 && bounds.h > 0)
		{
			map->chunks = Array2D<ecs::Entity>(bounds);
			world.entities.ForEach<TerrainChunk>([map](auto entity, const TerrainChunk& chunk){
				map->chunks[chunk.position] = entity;
			});
		}
	}

	std::vector<Point> GetNotLoadedChunksInArea(World& world, Rect chunks_area)
	{
		std::vector<Point> result;
		const auto* map = world.globals.Get<ChunkMap>();

		for (Point pt : utils::rect_points(chunks_area))
		{
			if (!map || !map->chunks.IndexIsValid(pt) || !map->chunks[pt]) {
				result.push_back(pt);
			}
		}

		return result;
	}
}