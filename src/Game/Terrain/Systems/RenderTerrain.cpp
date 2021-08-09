#include "RenderTerrain.h"

#include "Game/World.h"
#include "Game/CoordSystems.h"
#include "Game/Terrain/TerrainData.h"

#include "Utils/RectPoints.h"

#include <map>
#include <format>

namespace Expanse::Game::Terrain
{
	struct TerrainVertex
	{
		FPoint position;	
		FPoint uv;
		glm::vec4 color; // TODO: use 4 bytes for color, not 4 floats
	};

	static const Render::VertexLayout TerrainVertexFormat = { sizeof(TerrainVertex),
	{
		{ Render::VertexElementUsage::POSITION, sizeof(TerrainVertex::position), offsetof(TerrainVertex, position), 4, false, false },
		{ Render::VertexElementUsage::TEXCOORD0, sizeof(TerrainVertex::uv), offsetof(TerrainVertex, uv), 4, false, false },
		{ Render::VertexElementUsage::COLOR, sizeof(TerrainVertex::color), offsetof(TerrainVertex, color), 4, false, false },
	} };

	struct TerrainChunkRenderData
	{
		Render::Mesh mesh;
		Render::Material material;
	};

	RenderCells::RenderCells(World& w, Render::IRenderer* r)
		: ISystem(w)
		, renderer(r)
	{
		terrain_material = renderer->CreateMaterial("content/materials/terrain.json");
	}

	void RenderCells::Update()
	{
		GenerateChunksRenderData();

		RenderChunks();
	}

	void RenderCells::GenerateChunksRenderData()
	{
		for (const auto ent : world.entities.GetEntitiesWith<TerrainChunk>())
		{
			if (!world.entities.HasComponent<TerrainChunkRenderData>(ent)) {
				GenerateChunkRenderData(ent);
			}
		};
	}

	using TerrainTextureSlots = std::vector<TerrainType>;

	// Distributes terrain types in chunk into material texture slots
	TerrainTextureSlots AllocateTerrainTextureSlots(const TerrainChunk& chunk)
	{
		TerrainTextureSlots types;
		for (const auto& cell : chunk.cells)
		{
			if (std::ranges::find(types, cell.type) == types.end()) {
				types.push_back(cell.type);
			}
		}
		types.resize(4, TerrainType::Grass);
		return types;
	}

	// Finds texture slot number in terrain material for particular terrain type
	int GetTerrainTextureSlot(TerrainType type, const TerrainTextureSlots& types)
	{
		return static_cast<int>(std::ranges::find(types, type) - types.begin());
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

	// Adds vertices with their world positions, and indices into provided vectors.
	// Returns a view of the added vertices for further processing
	//
	auto EmitCellVertices(std::vector<TerrainVertex>& vertices, std::vector<uint16_t>& indices, Point cell_pos)
	{
		static constexpr std::array<FPoint, 8> cell_vertices = {{
			{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
			{0.2f, 0.2f}, {0.8f, 0.2f}, {0.8f, 0.8f}, {0.2f, 0.8f},
		}};
		static constexpr uint16_t cell_indices[] = { 0,1,4, 4,1,5, 5,1,2, 5,2,6, 6,2,3, 6,3,7, 7,3,0, 0,4,7, 4,5,7, 7,5,6 };

		// Write indices
		auto rebase_idx = [base = static_cast<uint16_t>(vertices.size())](uint16_t idx){ return base + idx; };
		auto rebased_indices = cell_indices | std::views::transform(rebase_idx);
		indices.insert(indices.end(), rebased_indices.begin(), rebased_indices.end());

		// Write vertices
		auto pos_to_vertex = [pos = FPoint{cell_pos}](FPoint vp) {
			return TerrainVertex{.position = pos + vp };
		};
		auto vertices_gen = cell_vertices | std::views::transform(pos_to_vertex);
		auto itr = vertices.insert(vertices.end(), vertices_gen.begin(), vertices_gen.end());

		// return view of inserted vertices
		return std::ranges::subrange{ itr, vertices.end() };
	}

	// Transforms vertex positions from world/local space into scene space for rendering
	//
	void TransformVerticesIntoSceneSpace(auto vertex_range)
	{
		for (auto& vtx : vertex_range) {
			vtx.position = Coords::WorldToScene(vtx.position);
		}
	}

	// Calculates vertex uvs based on their world/local positions
	//
	void CalcVertexUVs(auto vertex_range, float cell_uv_size)
	{
		for (auto& vtx : vertex_range) {
			vtx.uv = vtx.position * cell_uv_size;
		}
	}

	// Generates chunk meshes and materials
	//
	void RenderCells::GenerateChunkRenderData(ecs::Entity ent)
	{
		auto *render_data = world.entities.AddComponent<TerrainChunkRenderData>(ent);
		auto* chunk = world.entities.GetComponent<TerrainChunk>(ent);

		assert(chunk);

		// Find all terrain types in chunk
		const auto terrain_slots = AllocateTerrainTextureSlots(*chunk);

		std::vector<TerrainVertex> vertices;
		std::vector<uint16_t> indices;

		for (Point cell_pos : views::rect_points{ chunk->cells.GetRect() })
		{
			// Emit vertices and indices
			auto cell_verts = EmitCellVertices(vertices, indices, cell_pos);

			// get texture slot number in material for this cell terrain type
			const auto type_idx = GetTerrainTextureSlot(chunk->cells[cell_pos].type, terrain_slots);

			// Calc vertex texture coordinates
			CalcVertexUVs(cell_verts, 2.0f / chunk->Size);

			// Calc vertex colors
			for (auto& vtx : cell_verts) {
				vtx.color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
				vtx.color[type_idx] = 1.0f;
			}

			// Transform vertex positions into scene space
			TransformVerticesIntoSceneSpace(cell_verts);
		}

		// create mesh
		auto mesh = renderer->CreateMesh();
		renderer->SetMeshVertices(mesh, vertices, TerrainVertexFormat);
		renderer->SetMeshIndices(mesh, indices);
		render_data->mesh = mesh;

		// create material
		Render::Material material = renderer->CreateMaterial(terrain_material);
		SetTerrainMaterialTextures(renderer, material, terrain_slots);
		render_data->material = material;
	}




	void RenderCells::RenderChunks()
	{
		world.entities.ForEach<TerrainChunkRenderData, TerrainChunk>([this](auto ent, const TerrainChunkRenderData& rdata, const TerrainChunk& chunk)
		{
			const auto world_pos = Coords::LocalToWorld(FPoint{0.0f, 0.0f}, chunk.position, world.world_origin, chunk.Size);
			const auto scene_pos = Coords::WorldToScene(world_pos);
			renderer->SetMaterialParameter(rdata.material, "chunk_pos", glm::vec2{ scene_pos.x, scene_pos.y });
			renderer->Draw(rdata.mesh, rdata.material);
		});
	}
}