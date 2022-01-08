#include "pch.h"

#include "TerrainHelpers.h"

#include "Components/TerrainData.h"
#include "Utils/Bounds.h"
#include "Utils/RectPoints.h"

namespace Expanse::Game::Terrain
{
	void UpdateChunkMap(World& world)
	{
		auto* map = world.globals.GetOrCreate<ChunkMap>();
		
		// Calculate bounding rect of all chunks
		utils::Bounds<int> bounds;
		for (const auto& chunk : world.entities.GetComponentArray<TerrainChunk>()) {
			bounds.Add(chunk.position);
		}
		for (const auto& chunk : world.entities.GetComponentArray<AsyncLoadingChunk>()) {
			bounds.Add(chunk.position);
		}
		const auto bounds_rect = bounds.ToRect();

		// Write entities into map
		if (bounds_rect.w > 0 && bounds_rect.h > 0)
		{
			map->chunks = Array2D<ecs::Entity>(bounds_rect);
			world.entities.ForEach<TerrainChunk>([map](auto entity, const TerrainChunk& chunk){
				map->chunks[chunk.position] = entity;
			});
			world.entities.ForEach<AsyncLoadingChunk>([map](auto entity, const AsyncLoadingChunk& chunk) {
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