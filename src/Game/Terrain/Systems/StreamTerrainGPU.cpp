#include "pch.h"

#include "StreamTerrainGPU.h"

#include "Game/World.h"
#include "Game/CoordSystems.h"
#include "Game/Terrain/Components/TerrainMesh.h"
#include "Utils/Logger/Logger.h"
#include "Utils/RectPoints.h"
#include "Game/Utils/NeighbourCells.h"

#include "TerrainMeshGenerator.h"

#include <map>

namespace Expanse::Game::Terrain
{
	namespace
	{
		Rect GetChunksAreaToLoad(World& world, Point window_size)
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

	LoadChunksToGPU::LoadChunksToGPU(World& w, Render::IRenderer* r)
		: ISystem(w)
		, renderer(r)
	{
		terrain_material = renderer->CreateMaterial("content/materials/terrain.json");
	}

	void SetTerrainMaterialTextures(Render::IRenderer* renderer, Render::Material material, const TerrainTextureSlots& terrain_slots)
	{
		static const std::map<TerrainType, std::string> terrain_textures = {
			{ TerrainType::Dirt, "content/textures/dirt.json" },
			{ TerrainType::Grass, "content/textures/grass.json" },
			{ TerrainType::Stones, "content/textures/stones.json" }
		};

		assert(terrain_slots.size() == 4u);

		for (size_t i : std::views::iota(0u, terrain_slots.size())) {
			renderer->SetMaterialParameter(material, std::format("tex{}", i), terrain_textures.at(terrain_slots[i]));
		}
	}

	void LoadChunksToGPU::UploadTerrainMeshData(TerrainMesh& rdata, const TerrainMeshData& data)
	{
		// create mesh
		if (!rdata.mesh.IsValid())
			rdata.mesh = renderer->CreateMesh();
		renderer->SetMeshVertices(rdata.mesh, data.vertices, TerrainVertexFormat);
		renderer->SetMeshIndices(rdata.mesh, data.indices);

		// create material
		if (!rdata.material.IsValid())
			rdata.material = renderer->CreateMaterial(terrain_material);
		SetTerrainMaterialTextures(renderer, rdata.material, data.tex_slots);
	}

	void LoadChunksToGPU::Update()
	{
		const auto gen_entities = GatherChunksToLoad();

		// Generate meshes for them

		for (const auto ent : gen_entities)
		{
			auto* chunk = world.entities.GetComponent<TerrainChunk>(ent);
			assert(chunk);

			auto* render_data = world.entities.GetComponent<TerrainMesh>(ent);
			if (!render_data) {
				chunk->use_count++;
				render_data = world.entities.AddComponent<TerrainMesh>(ent);
			}

			const auto mesh_data = GenerateTerrainMesh(world, chunk->position);
			UploadTerrainMeshData(*render_data, mesh_data);
		};
	}

	std::vector<ecs::Entity> LoadChunksToGPU::GatherChunksToLoad() const
	{
		std::vector<ecs::Entity> gen_entities;

		auto* map = world.globals.Get<ChunkMap>();
		if (!map)
			return gen_entities;

		const auto visible_area = GetChunksAreaToLoad(world, renderer->GetWindowSize());
		const auto load_area = Intersection(map->chunks.GetRect(), visible_area);

		Array2D<bool> load_map{ load_area, false };

		// gather not loaded chunks in view
		world.entities.ForEach<TerrainChunk>([this, &load_map](auto ent, const TerrainChunk& chunk)
		{
			if (!world.entities.HasComponent<TerrainMesh>(ent) && load_map.IndexIsValid(chunk.position)) {
				load_map[chunk.position] = true;
			}
		});

		// gather chunks to update
		world.entities.ForEach<Event::ChunkLoaded, TerrainChunk>([&load_map](auto ent, const Event::ChunkLoaded&, const TerrainChunk& chunk)
		{
			for (Point off : Offset::Neighbors8) {
				const Point pos = chunk.position + off;
				if (load_map.IndexIsValid(pos)) {
					load_map[pos] = true;
				}
			}
		});

		// convert chunk map to entities list
		for (Point pt : utils::rect_points(load_map.GetRect()))
		{
			const auto ent = map->chunks[pt];
			if (load_map[pt] && ent) {
				gen_entities.push_back(ent);
			}
		}

		return gen_entities;
	}

	/*************************************************************************************************/

	UnloadChunksFromGPU::UnloadChunksFromGPU(World& w, Render::IRenderer* r)
		: ISystem(w)
		, renderer(r)
	{
	}

	void UnloadChunksFromGPU::Update()
	{
		const auto visible_area = GetChunksAreaToLoad(world, renderer->GetWindowSize());

		std::vector<ecs::Entity> freed_chunks;
		world.entities.ForEach<TerrainMesh, TerrainChunk>([this, &freed_chunks, visible_area](auto ent, const TerrainMesh& rdata, TerrainChunk& chunk)
		{
			if (!Contains(visible_area, chunk.position))
			{
				renderer->FreeMaterial(rdata.material);
				renderer->FreeMesh(rdata.mesh);

				freed_chunks.push_back(ent);

				chunk.use_count--;
			}
		});

		for (auto ent : freed_chunks) {
			world.entities.RemoveComponent<TerrainMesh>(ent);
		}
	}

	/*************************************************************************************************/

	
}