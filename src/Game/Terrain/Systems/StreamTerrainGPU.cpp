#include "StreamTerrainGPU.h"

#include "Game/World.h"
#include "Game/CoordSystems.h"
#include "Game/Terrain/Components/TerrainMesh.h"
#include "Utils/Logger/Logger.h"

#include "TerrainMeshGenerator.h"

#include <map>
#include <ranges>

namespace Expanse::Game::Terrain
{
	namespace
	{
		FRect ChunkSceneBounds(Point chunk_pos, Point world_origin)
		{
			const auto chunk_cell_rect = Coords::LocalToCell(TerrainChunk::Area, chunk_pos, TerrainChunk::Size);
			const auto chunk_world_rect = Coords::CellToWorld(chunk_cell_rect, world_origin);
			return Coords::WorldRectSceneBounds(chunk_world_rect);
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
			{ TerrainType::Grass, "content/textures/grass.json" }
		};

		assert(terrain_slots.size() == 4u);

		for (size_t i : std::views::iota(0u, terrain_slots.size())) {
			renderer->SetMaterialParameter(material, std::format("tex{}", i), terrain_textures.at(terrain_slots[i]));
		}
	}

	TerrainMesh LoadChunksToGPU::UploadTerrainMeshData(const TerrainMeshData& data)
	{
		// create mesh
		auto mesh = renderer->CreateMesh();
		renderer->SetMeshVertices(mesh, data.vertices, TerrainVertexFormat);
		renderer->SetMeshIndices(mesh, data.indices);

		// create material
		Render::Material material = renderer->CreateMaterial(terrain_material);
		SetTerrainMaterialTextures(renderer, material, data.tex_slots);

		return { mesh, material };
	}

	void LoadChunksToGPU::Update()
	{
		// Gather all chunks we need to render, that dont have associated meshes yet

		const auto window_rect = FRect{ renderer->GetWindowRect() };
		const auto view_rect = Centralized(window_rect) / world.camera_scale + world.camera_pos;
		ScaleFromCenter(view_rect, 1.1f);

		std::vector<ecs::Entity> gen_entities;
		world.entities.ForEach<TerrainChunk>([this, view_rect, &gen_entities](auto ent, const TerrainChunk& chunk)
		{
			if (!world.entities.HasComponent<TerrainMesh>(ent))
			{
				const FRect chunk_view = ChunkSceneBounds(chunk.position, world.world_origin);
				if (Intersects(chunk_view, view_rect)) {
					gen_entities.push_back(ent);
				}
			}
		});

		// Generate meshes for them

		for (const auto ent : gen_entities)
		{
			auto* render_data = world.entities.AddComponent<TerrainMesh>(ent);
			auto* chunk = world.entities.GetComponent<TerrainChunk>(ent);

			assert(chunk);

			chunk->use_count++;

			const auto mesh_data = GenerateTerrainMesh(*chunk);
			*render_data = UploadTerrainMeshData(mesh_data);
		};
	}

	/*************************************************************************************************/

	UnloadChunksFromGPU::UnloadChunksFromGPU(World& w, Render::IRenderer* r)
		: ISystem(w)
		, renderer(r)
	{
	}

	void UnloadChunksFromGPU::Update()
	{
		const auto window_rect = FRect{ renderer->GetWindowRect() };
		const auto view_rect = Centralized(window_rect) / world.camera_scale + world.camera_pos;
		ScaleFromCenter(view_rect, 2.0f);

		std::vector<ecs::Entity> freed_chunks;
		world.entities.ForEach<TerrainMesh, TerrainChunk>([this, &freed_chunks, view_rect](auto ent, const TerrainMesh& rdata, TerrainChunk& chunk)
		{
			const auto chunk_view = ChunkSceneBounds(chunk.position, world.world_origin);
			if (!Intersects(chunk_view, view_rect))
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