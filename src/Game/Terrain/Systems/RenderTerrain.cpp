#include "RenderTerrain.h"

#include "Game/World.h"
#include "Game/CoordSystems.h"
#include "Game/Terrain/TerrainData.h"

#include "Utils/RectPoints.h"
#include "Utils/Utils.h"

#include <map>
#include <format>
#include <numeric>

#include "Game/Utils/NeighbourCells.h"

namespace Expanse::Game::Terrain
{
	// Cell geometry configuration
	namespace CellGeometry
	{
		// How much does one unit of height takes in scene coordinates
		static constexpr float UnitHeight = 0.1f;

		/*
		* Vertices configuration
		* 1 - 2 - 3
		* | \ | / |
		* 8 - 0 - 4
		* | / | \ |
		* 7 - 6 - 5
		*/

		static constexpr size_t CellVertexCount = 9;

		static constexpr FPoint vertices[CellVertexCount] = {
			{ 0.5f, 0.5f },
			{ 0.0f, 1.0f }, { 0.5f, 1.0f },
			{ 1.0f, 1.0f }, { 1.0f, 0.5f },
			{ 1.0f, 0.0f }, { 0.5f, 0.0f },
			{ 0.0f, 0.0f }, { 0.0f, 0.5f }
		};

		static constexpr uint16_t indices[] = {
			0,2,1, 0,3,2, 0,4,3, 0,5,4, 0,6,5, 0,7,6, 0,8,7, 0,1,8
		};

		using VertexColorWeights = Neighbours<uint8_t>;

		static constexpr VertexColorWeights color_weights[CellVertexCount] = {
			{ 0, 0, 0, 0, 255, 0, 0, 0, 0 },   // center
			{ 63, 64, 0, 64, 64, 0, 0, 0, 0 }, // left top
			{ 0, 127, 0, 0, 128, 0, 0, 0, 0 }, // up
			{ 0, 64, 63, 0, 64, 64, 0, 0, 0 }, // right top
			{ 0, 0, 0, 0, 128, 127, 0, 0, 0 }, // right
			{ 0, 0, 0, 0, 64, 64, 0, 64, 63 }, // right down
			{ 0, 0, 0, 0, 128, 0, 0, 127, 0 }, // down
			{ 0, 0, 0, 64, 64, 0, 63, 64, 0}, // left bottom
			{ 0, 0, 0, 127, 128, 0, 0, 0, 0}, // left
		};


		using VertexHeightWeights = Neighbours<float>;

		static constexpr VertexHeightWeights height_weights[CellVertexCount] = {
			{ 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }, // center
			{ 0.25f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.0f, 0.0f, 0.0f}, // left top
			{ 0.0f, 0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f}, // up
			{ 0.0f, 0.25f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.0f, 0.0f}, // right top
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f }, // right
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f, 0.25f }, // right down
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f, 0.0f }, // down
			{ 0.0f, 0.0f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f}, // left bottom
			{ 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f}, // left			
		};
	}


	struct TerrainVertex
	{
		FPoint position;	
		FPoint uv;
		Render::Color color;
	};

	static const Render::VertexLayout TerrainVertexFormat = { sizeof(TerrainVertex),
	{
		{ Render::VertexElementUsage::POSITION, sizeof(TerrainVertex::position), offsetof(TerrainVertex, position), 4, false, false },
		{ Render::VertexElementUsage::TEXCOORD0, sizeof(TerrainVertex::uv), offsetof(TerrainVertex, uv), 4, false, false },
		{ Render::VertexElementUsage::COLOR, sizeof(TerrainVertex::color), offsetof(TerrainVertex, color), 1, true, false },
	} };

	RenderCells::RenderCells(World& w, Render::IRenderer* r)
		: ISystem(w)
		, renderer(r)
		, helper(w)
	{
		terrain_material = renderer->CreateMaterial("content/materials/terrain.json");
	}

	void RenderCells::Update()
	{
		helper.Update();
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
	std::pair<Array2D<uint8_t>, TerrainTextureSlots> AllocateTerrainTextureSlots(const Array2D<TerrainType>& chunk)
	{
		std::pair<Array2D<uint8_t>, TerrainTextureSlots> result;

		// find all types, assign them indices
		auto& types = result.second;
		for (const auto& cell_type : chunk) {
			if (!utils::contains(types, cell_type)) {
				types.push_back(cell_type);
			}
		}
		types.resize(4, TerrainType::Grass);
		
		// create map of all cells terrain slots
		auto& arr = result.first;
		arr = Array2D<uint8_t>{ chunk.GetRect() };
		auto get_index = [&types](const auto& cell_type){ return static_cast<uint8_t>(std::ranges::find(types, cell_type) - types.begin()); };
		std::transform(chunk.begin(), chunk.end(), arr.begin(), get_index);

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

	void EmitIndices(size_t vertex_count, std::vector<uint16_t>& indices)
	{
		auto rebase_idx = [base = static_cast<uint16_t>(vertex_count)](uint16_t idx){ return base + idx; };
		auto rebased_indices = CellGeometry::indices | std::views::transform(rebase_idx);
		indices.insert(indices.end(), rebased_indices.begin(), rebased_indices.end());
	}

	// Adds vertices for the cell into vertex array, return a view of the added vertices for further processing
	//
	auto EmitCellVertices(std::vector<TerrainVertex>& vertices)
	{
		auto itr = vertices.insert(vertices.end(), CellGeometry::CellVertexCount, TerrainVertex{});
		return std::ranges::subrange{ itr, vertices.end() };
	}

	std::vector<glm::vec3> CalcVertexPositions(Point cell_pos, const Array2D<float>& heights)
	{
		auto pos_to_vertex = [pos = FPoint{ cell_pos }](FPoint vp) {
			return glm::vec3{ pos.x + vp.x, pos.y + vp.y, 0.0f };
		};
		auto vertices_gen = CellGeometry::vertices | std::views::transform(pos_to_vertex);
		std::vector<glm::vec3> positions{ vertices_gen.begin(), vertices_gen.end() };


		const auto neighbours = SelectNeighbours(cell_pos, heights);
		utils::for_each_zipped(positions, CellGeometry::height_weights, [neighbours](auto& pos, const auto& weights)
		{
			pos.z = std::inner_product(weights.begin(), weights.end(), neighbours.begin(), 0.0f);
		});

		return positions;
	}

	// Transforms vertex positions from world/local space into scene space for rendering
	//
	void WriteVertexPositions(auto vertex_range, const std::vector<glm::vec3>& positions)
	{
		utils::for_each_zipped(vertex_range, positions, [](auto& vtx, glm::vec3 pos) {
			vtx.position = Coords::WorldToScene(FPoint{ pos.x, pos.y });
			vtx.position.y += pos.z;
		});
	}

	// Calculates vertex uvs based on their world/local positions
	//
	void WriteVertexUVs(auto vertex_range, float cell_uv_size, const std::vector<glm::vec3>& positions)
	{
		utils::for_each_zipped(vertex_range, positions, [cell_uv_size](auto& vtx, glm::vec3 pos) {
			vtx.uv = FPoint{ pos.x, pos.y } * cell_uv_size;
		});
	}

	// Landscape types blending calculations

	Render::Color CalcVertexColor(const CellGeometry::VertexColorWeights& weights, const Neighbours<uint8_t>& neighbours)
	{
		std::array<uint8_t, 4> color = { 0, 0, 0, 0} ;
		utils::for_each_zipped(weights, neighbours, [&color](uint8_t w, auto t) { color[t] += w; });
		return Render::Color{ color };
	}

	void WriteVertexColors(auto vertex_range, Point cell, const Array2D<uint8_t>& slot_map)
	{
		// color vertices with blending weights
		const auto neighbours = SelectNeighbours(cell, slot_map);
		auto [v_itr, w_itr] = utils::for_each_zipped(vertex_range, CellGeometry::color_weights, [neighbours](auto& vtx, const auto& weights)
		{
			vtx.color = CalcVertexColor(weights, neighbours);
		});
	}

	// Generates chunk meshes and materials
	//
	void RenderCells::GenerateChunkRenderData(ecs::Entity ent)
	{
		auto *render_data = world.entities.AddComponent<TerrainChunkRenderData>(ent);
		auto* chunk = world.entities.GetComponent<TerrainChunk>(ent);

		assert(chunk);

		const auto chunk_rect = chunk->cells.GetRect();

		// Create heights map for chunks
		Array2D<float> heights{ Inflated(chunk_rect, 1 , 1) };
		for (const auto pt : utils::rect_points(heights.GetRect()))
		{
			if (chunk->cells.IndexIsValid(pt)) {
				// cell in this chunk
				heights[pt] = chunk->cells[pt].height * CellGeometry::UnitHeight;
			}
			else {
				// cell in neighboring chunk
				const auto ncell = Coords::LocalToCell(pt, chunk->position, chunk->Size);
				if (auto* cell = helper.GetCell(ncell)) {
					// we have neighbour chunk loaded, ok
					heights[pt] = cell->height * CellGeometry::UnitHeight;
				}
				else {
					// no data, assume same type as on this chunks border
					const auto clamped_pt = Clamp(pt, chunk_rect);
					heights[pt] = chunk->cells[clamped_pt].height * CellGeometry::UnitHeight;
				}
			}
		}

		// Create terrain types map for chunk
		Array2D<TerrainType> types{ Inflated(chunk_rect, 1, 1) };
		for (const auto pt : utils::rect_points(types.GetRect()))
		{
			if (chunk->cells.IndexIsValid(pt)) {
				// cell in this chunk
				types[pt] = chunk->cells[pt].type;
			} else {
				// cell in neighboring chunk
				const auto ncell = Coords::LocalToCell(pt, chunk->position, chunk->Size);
				if (auto* cell = helper.GetCell(ncell)) {
					// we have neighbour chunk loaded, ok
					types[pt] = cell->type;
				} else {
					// no data, assume same type as on this chunks border
					const auto clamped_pt = Clamp(pt, chunk_rect);
					types[pt] = chunk->cells[clamped_pt].type;
				}
			}
		}

		// Distribute terrain types to textures slots
		const auto& [slot_map, slots] = AllocateTerrainTextureSlots(types);

		// Create vertex and index buffers
		const auto chunk_cells_count = chunk->Size * chunk->Size;
		std::vector<TerrainVertex> vertices;
		std::vector<uint16_t> indices;
		vertices.reserve(std::size(CellGeometry::vertices) * chunk_cells_count);
		indices.reserve(std::size(CellGeometry::indices) * chunk_cells_count);


		for (Point cell_pos : utils::rect_points_from_top(chunk_rect))
		{
			// Append indices and vertices
			EmitIndices(vertices.size(), indices);
			auto cell_verts = EmitCellVertices(vertices);

			// Calculate 3D positions of vertices in world
			const auto positions = CalcVertexPositions(cell_pos, heights);

			WriteVertexPositions(cell_verts, positions);
			WriteVertexUVs(cell_verts, 0.5f, positions);
			WriteVertexColors(cell_verts, cell_pos, slot_map);
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