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
				auto ent = world.entities.CreateEntity();
				auto* chunk = world.entities.AddComponent<TerrainChunk>(ent, chunk_pos);

				for (auto& loader : loaders) {
					if (loader->LoadChunk(*chunk))
						break;
				}

				world.entities.AddComponent<Event::ChunkLoaded>(ent);
			}

			loaded_area = req_area;
			UpdateChunkMap(world);
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