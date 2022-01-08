#include "pch.h"

#include "GenerateTerrain.h"

#include "Game/World.h"
#include "Game/CoordSystems.h"

#include "Utils/Random.h"
#include "Utils/RectPoints.h"
#include "Utils/Utils.h"

#include <set>

#include "Game/Terrain/Systems/ProceduralTerrain.h"
#include "Game/Terrain/TerrainHelpers.h"

namespace Expanse::Game::Terrain
{
	namespace
	{
		Rect GetMapAreaToLoad(World& world, Point window_size)
		{
			const auto window_rect = FRect{ 0, 0, static_cast<float>(window_size.x), static_cast<float>(window_size.y) };
			const auto view_rect = Centralized(window_rect) / world.camera_scale + world.camera_pos;
			ScaleFromCenter(view_rect, 2.0f);

			const auto world_rect = Coords::SceneRectWorldBounds(view_rect);
			const auto cell_rect = Coords::WorldRectCellBounds(world_rect, world.world_origin);
			const auto chunks_area = Coords::CellRectChunkBounds(cell_rect, TerrainChunk::Size);
			return chunks_area;
		}
	}

	LoadChunks::LoadChunks(World& w, uint32_t seed, Point wnd_size)
		: ISystem(w)
		, window_size(wnd_size)
	{
		AddLoader<TerrainLoader_Procedural>(seed);
	}

	ITerrainLoader* LoadChunks::GetLoaderForChunk(Point chunk_pos)
	{
		auto can_load = [chunk_pos](const auto& loader) { return loader->HasChunk(chunk_pos); };
		auto itr = std::ranges::find_if(loaders, can_load);
		return (itr != loaders.end()) ? itr->get() : nullptr;
	}

	void LoadChunks::Update()
	{
		// Clear loaded events
		const auto ents = world.entities.GetEntitiesWith<Event::ChunkLoaded>();
		for (auto ent : ents) {
			world.entities.RemoveComponent<Event::ChunkLoaded>(ent);
		}

		// Load chunks
		const auto req_area = GetMapAreaToLoad(world, window_size);
		if (loaded_area != req_area)
		{
			const auto chunks_to_load = GetNotLoadedChunksInArea(world, req_area);
			for (const auto chunk_pos : chunks_to_load)
			{
				// find loader to use
				if (auto* loader = GetLoaderForChunk(chunk_pos))
				{
					auto ent = world.entities.CreateEntity();
					auto* loading_chunk = world.entities.AddComponent<AsyncLoadingChunk>(ent, chunk_pos);
					loading_chunk->cells = loader->LoadChunk(chunk_pos);
				}
			}

			loaded_area = req_area;
			UpdateChunkMap(world);
		}

		// Process loading chunks
		std::vector<ecs::Entity> loaded_chunks;
		world.entities.ForEach<AsyncLoadingChunk>([&, this](auto ent, AsyncLoadingChunk& async_chunk)
		{
			const auto status = async_chunk.cells.wait_for(std::chrono::seconds(0));
			if (status == std::future_status::ready)
			{
				auto* chunk = world.entities.AddComponent<TerrainChunk>(ent, async_chunk.position);
				chunk->cells = async_chunk.cells.get();
				
				world.entities.AddComponent<Event::ChunkLoaded>(ent);

				loaded_chunks.push_back(ent);
			}
		});
		for (auto ent : loaded_chunks) {
			world.entities.RemoveComponent<AsyncLoadingChunk>(ent);
		}
	}

	/*************************************************************************************************/

	UnloadChunks::UnloadChunks(World& w, Point wnd_size)
		: ISystem(w)
		, window_size(wnd_size)
	{}

	void UnloadChunks::Update()
	{
		const auto req_area = GetMapAreaToLoad(world, window_size);

		std::vector<ecs::Entity> free_chunks;
		world.entities.ForEach<TerrainChunk>([req_area, &free_chunks](auto ent, const TerrainChunk& chunk)
		{
			if (chunk.use_count <= 0 && !Contains(req_area, chunk.position)) {
				free_chunks.push_back(ent);
			}
		});

		if (!free_chunks.empty())
		{
			world.entities.DestroyEntities(free_chunks);
			UpdateChunkMap(world);
		}
	}
}