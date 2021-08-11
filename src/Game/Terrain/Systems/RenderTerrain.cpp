#include "RenderTerrain.h"

#include "Game/World.h"
#include "Game/CoordSystems.h"
#include "Game/Terrain/TerrainData.h"

#include "Utils/RectPoints.h"
#include "Utils/Utils.h"

#include <map>
#include <format>

#include "Game/Utils/NeighbourCells.h"

namespace Expanse::Game::Terrain
{
	// Cell geometry configuration
	namespace CellGeometry
	{
		constexpr float facet = 0.35f;

		static_assert(facet >= 0.0f && facet <= 0.5f);

		static constexpr FPoint vertices[] = {
			// outer ring
			{0.0f, 0.0f}, {0.0f, facet}, {0.0f, 1.0f - facet},
			{0.0f, 1.0f}, {facet, 1.0f}, {1.0f - facet, 1.0f},
			{1.0f, 1.0f}, {1.0f, 1.0f - facet}, {1.0f, facet},
			{1.0f, 0.0f}, {1.0f - facet, 0.0f}, {facet, 0.0f},
			// inner ring
			{facet, facet}, {facet, 1.0f - facet}, {1.0f - facet, 1.0f - facet}, {1.0f - facet, facet},
		};

		static constexpr uint16_t indices[] = {
			// corners
			1,0,12, 12,0,11,
			3,2,13, 3,13,4,
			5,14,6, 6,14,7,
			8,15,9, 9,15,10,

			// sides
			2,1,13, 13,1,12,
			4,13,14, 4,14,5,
			7,14,15, 7,15,8,
			12,11,10, 12,10,15,

			//center
			13,12,14, 14,12,15
		};

		using VertexColorWeights = Neighbours<float>;

		static constexpr VertexColorWeights color_weights[] = {
			{ 0.0f, 0.0f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f}, // left bottom
			{ 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f}, // left
			{ 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f}, // left
			{ 0.25f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.0f, 0.0f, 0.0f}, // left top
			{ 0.0f, 0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f}, // up
			{ 0.0f, 0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f}, // up
			{ 0.0f, 0.25f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.0f, 0.0f}, // right top
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f }, // right
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f }, // right
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f, 0.25f }, // right down
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f, 0.0f }, // down
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f, 0.0f }, // down
		};
	}


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
	std::pair<Array2D<uint8_t>, TerrainTextureSlots> AllocateTerrainTextureSlots(const TerrainChunk& chunk)
	{
		std::pair<Array2D<uint8_t>, TerrainTextureSlots> result;

		// find all types, assign them indices
		auto& types = result.second;
		for (const auto& cell : chunk.cells)
		{
			if (!utils::contains(types, cell.type)) {
				types.push_back(cell.type);
			}
		}
		types.resize(4, TerrainType::Grass);
		
		// create map of all cells terrain slots
		auto& arr = result.first;
		arr = Array2D<uint8_t>{ chunk.cells.GetRect() };
		auto get_index = [&types](const auto& cell){ return static_cast<uint8_t>(std::ranges::find(types, cell.type) - types.begin()); };
		std::transform(chunk.cells.begin(), chunk.cells.end(), arr.begin(), get_index);

		return result;
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
		// Write indices
		auto rebase_idx = [base = static_cast<uint16_t>(vertices.size())](uint16_t idx){ return base + idx; };
		auto rebased_indices = CellGeometry::indices | std::views::transform(rebase_idx);
		indices.insert(indices.end(), rebased_indices.begin(), rebased_indices.end());

		// Write vertices
		auto pos_to_vertex = [pos = FPoint{cell_pos}](FPoint vp) {
			return TerrainVertex{.position = pos + vp };
		};
		auto vertices_gen = CellGeometry::vertices | std::views::transform(pos_to_vertex);
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

	glm::vec4 CalcVertexColor(const CellGeometry::VertexColorWeights& weights, const Neighbours<uint8_t>& neighbours)
	{
		glm::vec4 color{ 0.0f };
		utils::for_each_zipped(weights, neighbours, [&color](float w, auto t) { color[t] += w; });
		return color;
	}

	void CalcVertexColors(auto vertex_range, Point cell, const Array2D<uint8_t>& slot_map)
	{
		// color vertices with blending weights
		const auto neighbours = SelectNeighbours(cell, slot_map, slot_map[cell]);
		auto [v_itr, w_itr] = utils::for_each_zipped(vertex_range, CellGeometry::color_weights, [neighbours](auto& vtx, const auto& weights)
		{
			vtx.color = CalcVertexColor(weights, neighbours);
		});

		// apply default color to remaining vertices
		glm::vec4 def_color{ 0.0f };
		def_color[neighbours[Offset::Center]] = 1.0f;
		for (auto& vtx : std::ranges::subrange(v_itr, vertex_range.end())) {
			vtx.color = def_color;
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
		const auto& [slot_map, slots] = AllocateTerrainTextureSlots(*chunk);

		// Create vertex and index buffers
		const auto chunk_cells_count = chunk->Size * chunk->Size;
		std::vector<TerrainVertex> vertices;
		std::vector<uint16_t> indices;
		vertices.reserve(std::size(CellGeometry::vertices) * chunk_cells_count);
		indices.reserve(std::size(CellGeometry::indices) * chunk_cells_count);



		for (Point cell_pos : utils::rect_points{ chunk->cells.GetRect() })
		{
			// Emit vertices and indices
			auto cell_verts = EmitCellVertices(vertices, indices, cell_pos);

			// Calc vertex texture coordinates
			CalcVertexUVs(cell_verts, 2.0f / chunk->Size);

			// Calc vertex colors
			CalcVertexColors(cell_verts, cell_pos, slot_map);

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
		SetTerrainMaterialTextures(renderer, material, slots);
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