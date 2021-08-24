#include "RenderTerrain.h"

#include "Game/World.h"
#include "Game/CoordSystems.h"
#include "Game/Terrain/TerrainData.h"

#include "Utils/RectPoints.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

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
		static constexpr float UnitHeight = 0.2f;

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

	LoadChunksToGPU::LoadChunksToGPU(World& w, Render::IRenderer* r)
		: ISystem(w)
		, renderer(r)
	{
		terrain_material = renderer->CreateMaterial("content/materials/terrain.json");
	}

	void LoadChunksToGPU::Update()
	{
		const auto window_rect = FRect{ renderer->GetWindowRect() };
		const auto view_rect = Centralized(window_rect) / world.camera_scale + world.camera_pos;
		ScaleFromCenter(view_rect, 1.1f);

		std::vector<ecs::Entity> gen_entities;
		world.entities.ForEach<TerrainChunk>([this, view_rect, &gen_entities](auto ent, const TerrainChunk& chunk)
		{
			if (!world.entities.HasComponent<TerrainChunkRenderData>(ent)) {
				const FRect chunk_view = Coords::ChunkSceneBounds(world.world_origin, chunk.position, TerrainChunk::Size);
				if (Intersects(chunk_view, view_rect)) {
					gen_entities.push_back(ent);
				}
			}
		});

		if (!gen_entities.empty())
		{
			for (const auto ent : gen_entities) {
				GenerateChunkRenderData(ent);
			};
		}
	}

	using TerrainTextureSlots = std::vector<TerrainType>;

	// Distributes terrain types in chunk into material texture slots
	std::pair<Array2D<uint8_t>, TerrainTextureSlots> AllocateTerrainTextureSlots(const TerrainChunk& chunk)
	{
		std::pair<Array2D<uint8_t>, TerrainTextureSlots> result;

		// find all types, assign them indices
		auto& types = result.second;
		for (const auto& cell : chunk.cells) {
			if (!utils::contains(types, cell.type)) {
				types.push_back(cell.type);
			}
		}
		types.resize(4, TerrainType::Grass);
		
		// create map of all cells terrain slots
		auto& arr = result.first;
		arr = Array2D<uint8_t>{ chunk.cells.GetRect() };
		auto get_index = [&types](const TerrainCell& cell){ return static_cast<uint8_t>(std::ranges::find(types, cell.type) - types.begin()); };
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

	std::vector<glm::vec3> CalcVertexPositions(Point cell_pos, const TerrainChunk& chunk)
	{
		auto pos_to_vertex = [pos = FPoint{ cell_pos }](FPoint vp) {
			return glm::vec3{ pos.x + vp.x, pos.y + vp.y, 0.0f };
		};
		auto vertices_gen = CellGeometry::vertices | std::views::transform(pos_to_vertex);
		std::vector<glm::vec3> positions{ vertices_gen.begin(), vertices_gen.end() };


		const auto neighbours = SelectNeighbours(cell_pos, chunk.cells, [](const TerrainCell& cell){
			return cell.height * CellGeometry::UnitHeight;
		});

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
	void LoadChunksToGPU::GenerateChunkRenderData(ecs::Entity ent)
	{
		auto *render_data = world.entities.AddComponent<TerrainChunkRenderData>(ent);
		auto* chunk = world.entities.GetComponent<TerrainChunk>(ent);

		assert(chunk);

		// Distribute terrain types to textures slots
		const auto& [slot_map, slots] = AllocateTerrainTextureSlots(*chunk);

		// Create vertex and index buffers
		const auto chunk_cells_count = chunk->Size * chunk->Size;
		std::vector<TerrainVertex> vertices;
		std::vector<uint16_t> indices;
		vertices.reserve(std::size(CellGeometry::vertices) * chunk_cells_count);
		indices.reserve(std::size(CellGeometry::indices) * chunk_cells_count);


		for (Point cell_pos : utils::rect_points_rt2lb(TerrainChunk::Area))
		{
			// Append indices and vertices
			EmitIndices(vertices.size(), indices);
			auto cell_verts = EmitCellVertices(vertices);

			// Calculate 3D positions of vertices in world
			const auto positions = CalcVertexPositions(cell_pos, *chunk);

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


		Log::message("Chunk {}:{} render data generated", chunk->position.x, chunk->position.y);
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
		world.entities.ForEach<TerrainChunkRenderData, TerrainChunk>([this, &freed_chunks, view_rect](auto ent, const TerrainChunkRenderData& rdata, const TerrainChunk& chunk)
		{
			const FRect chunk_view = Coords::ChunkSceneBounds(world.world_origin, chunk.position, TerrainChunk::Size);
			if (!Intersects(chunk_view, view_rect))
			{
				renderer->FreeMaterial(rdata.material);
				renderer->FreeMesh(rdata.mesh);

				freed_chunks.push_back(ent);
				Log::message("Chunk {}:{} unloaded from GPU", chunk.position.x, chunk.position.y);
			}
		});

		for (auto ent : freed_chunks) {
			world.entities.RemoveComponent<TerrainChunkRenderData>(ent);
		}
	}

	/*************************************************************************************************/

	RenderChunks::RenderChunks(World& w, Render::IRenderer* r)
		: ISystem(w)
		, renderer(r)
	{
	}

	void RenderChunks::Update()
	{
		// Gather all chunks
		std::vector<std::pair<Point, TerrainChunkRenderData>> chunks;
		world.entities.ForEach<TerrainChunkRenderData, TerrainChunk>([&chunks](auto ent, const TerrainChunkRenderData& rdata, const TerrainChunk& chunk)
		{
			chunks.emplace_back(chunk.position, rdata);
		});

		// Sort
		auto comp = [](const auto& ch1, const auto& ch2){ return (ch1.first.x + ch1.first.y) > (ch2.first.x + ch2.first.y); };
		std::ranges::sort(chunks, comp);

		// Render
		for (const auto [pos, data] : chunks)
		{
			const auto world_pos = Coords::LocalToWorld(FPoint{ 0.0f, 0.0f }, pos, world.world_origin, TerrainChunk::Size);
			const auto scene_pos = Coords::WorldToScene(world_pos);
			renderer->SetMaterialParameter(data.material, "chunk_pos", glm::vec2{ scene_pos.x, scene_pos.y });
			renderer->Draw(data.mesh, data.material);
		}
	}
}