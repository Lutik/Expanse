#include "pch.h"

#include "TerrainMeshGenerator.h"

#include "Game/Utils/NeighbourCells.h"
#include "Utils/Utils.h"
#include "Utils/Async.h"
#include "Game/CoordSystems.h"

#include <map>
#include <format>
#include <numeric>

namespace Expanse::Game::Terrain
{
	// Cell geometry configuration
	namespace CellGeometry
	{
		// How much does one unit of height takes in scene coordinates
		static constexpr float UnitHeight = 0.15f;

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
			{ 0, 0, 0, 64, 64, 0, 63, 64, 0 }, // left down
			{ 0, 0, 0, 127, 128, 0, 0, 0, 0 }, // left
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
			{ 0.0f, 0.0f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f}, // left down
			{ 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f}, // left			
		};
	}


	auto EmitCellVertices(std::vector<TerrainVertex>& vertices)
	{
		auto itr = vertices.insert(vertices.end(), CellGeometry::CellVertexCount, TerrainVertex{});
		return std::ranges::subrange{ itr, vertices.end() };
	}

	// Writes precalculated positions into vertex array, transforming them from world/local space into scene space for rendering
	//
	void WriteVertexPositions(auto vertex_range, const std::vector<glm::vec3>& positions)
	{
		utils::for_each_zipped(vertex_range, positions, [](auto& vtx, glm::vec3 pos) {
			vtx.position = Coords::WorldToScene(FPoint{ pos.x, pos.y });
			vtx.position.y += pos.z;
		});
	}

	// Writes precalculated normals into vertex array
	//
	void WriteVertexNormals(auto vertex_range, const std::vector<glm::vec3>& normals)
	{
		utils::for_each_zipped(vertex_range, normals, [](auto& vtx, glm::vec3 normal) {
			vtx.normal = normal;
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

	void EmitIndices(size_t vertex_count, std::vector<uint16_t>& indices)
	{
		auto rebase_idx = [base = static_cast<uint16_t>(vertex_count)](uint16_t idx){ return base + idx; };
		auto rebased_indices = CellGeometry::indices | std::views::transform(rebase_idx);
		indices.insert(indices.end(), rebased_indices.begin(), rebased_indices.end());
	}

	std::vector<glm::vec3> CalcVertexPositions(Point cell_pos, const Array2D<TerrainCell>& chunk_cells)
	{
		auto pos_to_vertex = [pos = FPoint{ cell_pos }](FPoint vp) {
			return glm::vec3{ pos.x + vp.x, pos.y + vp.y, 0.0f };
		};
		auto vertices_gen = CellGeometry::vertices | std::views::transform(pos_to_vertex);
		std::vector<glm::vec3> positions{ vertices_gen.begin(), vertices_gen.end() };


		const auto neighbours = SelectNeighbours(cell_pos, chunk_cells, [](const TerrainCell& cell) {
			return cell.height * CellGeometry::UnitHeight;
		});

		utils::for_each_zipped(positions, CellGeometry::height_weights, [&neighbours](auto& pos, const auto& weights)
		{
			pos.z = std::inner_product(weights.begin(), weights.end(), neighbours.begin(), 0.0f);
		});

		return positions;
	}

	glm::vec3 CalcVertexNormal(const Neighbours<float>& heights, size_t vtx_index)
	{
		glm::vec3 n{ 0.0f, 0.0f, 2.0f };

		if (vtx_index == 0) // center
		{
			n.x = heights[Offset::Left] - heights[Offset::Right];
			n.y = heights[Offset::Down] - heights[Offset::Up];
		}
		else if (vtx_index == 1) // left up
		{
			n.x = heights[Offset::LeftUp] + heights[Offset::Left] - heights[Offset::Up] - heights[Offset::Center];
			n.y = heights[Offset::Left] + heights[Offset::Center] - heights[Offset::LeftUp] - heights[Offset::Up];
		}
		else if (vtx_index == 2) // up
		{
			n.x = (heights[Offset::Left] + heights[Offset::LeftUp] - heights[Offset::Right] - heights[Offset::RightUp]) * 0.5f;
			n.y = (heights[Offset::Center] - heights[Offset::Up]) * 2.0f;
		}
		else if (vtx_index == 3) // right up
		{
			n.x = heights[Offset::Center] + heights[Offset::Up] - heights[Offset::Right] - heights[Offset::RightUp];
			n.y = heights[Offset::Center] + heights[Offset::Right] - heights[Offset::Up] - heights[Offset::RightUp];
		}
		else if (vtx_index == 4) // right
		{
			n.x = (heights[Offset::Center] - heights[Offset::Right]) * 2.0f;
			n.y = (heights[Offset::Down] + heights[Offset::RightDown] - heights[Offset::Up] - heights[Offset::RightUp]) * 0.5f;
		}
		else if (vtx_index == 5) // right down
		{
			n.x = heights[Offset::Center] + heights[Offset::Down] - heights[Offset::Right] - heights[Offset::RightDown];
			n.y = heights[Offset::Down] + heights[Offset::RightDown] - heights[Offset::Center] - heights[Offset::Right];
		}
		else if (vtx_index == 6) // down
		{
			n.x = (heights[Offset::Left] + heights[Offset::LeftDown] - heights[Offset::Right] - heights[Offset::RightDown]) * 0.5f;
			n.y = (heights[Offset::Down] - heights[Offset::Center]) * 2.0f;
		}
		else if (vtx_index == 7) // left down
		{
			n.x = heights[Offset::Left] + heights[Offset::LeftDown] - heights[Offset::Center] - heights[Offset::Down];
			n.y = heights[Offset::LeftDown] + heights[Offset::Down] - heights[Offset::Center] - heights[Offset::Left];
		}
		else if (vtx_index == 8) // left
		{
			n.x = (heights[Offset::Left] - heights[Offset::Center]) * 2.0f;
			n.y = (heights[Offset::LeftDown] + heights[Offset::Down] - heights[Offset::LeftUp] - heights[Offset::Up]) * 0.5f;
		}

		return glm::normalize(n);
	}

	std::vector<glm::vec3> CalcVertexNormals(Point cell_pos, const Array2D<TerrainCell>& chunk_cells)
	{
		const auto nheights = SelectNeighbours(cell_pos, chunk_cells, [](const TerrainCell& cell) {
			return cell.height * CellGeometry::UnitHeight;
		});

		std::vector<glm::vec3> normals(CellGeometry::CellVertexCount, glm::vec3{ 0.0f, 0.0f, 1.0f });

		for (size_t idx = 0; idx < CellGeometry::CellVertexCount; ++idx) {
			normals[idx] = CalcVertexNormal(nheights, idx);
		}

		return normals;
	}

	// Distributes terrain types in chunk into material texture slots
	std::pair<Array2D<uint8_t>, TerrainTextureSlots> AllocateTerrainTextureSlots(const Array2D<TerrainCell>& chunk_cells)
	{
		std::pair<Array2D<uint8_t>, TerrainTextureSlots> result;

		// find all types, assign them indices
		auto& types = result.second;
		for (const auto& cell : chunk_cells) {
			if (!utils::contains(types, cell.type)) {
				types.push_back(cell.type);
			}
		}
		types.resize(4, TerrainType::Grass);

		// create map of all cells terrain slots
		auto& arr = result.first;
		arr = Array2D<uint8_t>{ chunk_cells.GetRect() };
		auto get_index = [&types](const TerrainCell& cell) { return static_cast<uint8_t>(std::ranges::find(types, cell.type) - types.begin()); };
		std::transform(chunk_cells.begin(), chunk_cells.end(), arr.begin(), get_index);

		return result;
	}

	// Landscape types blending calculations

	Render::Color CalcVertexColor(const CellGeometry::VertexColorWeights& weights, const Neighbours<uint8_t>& neighbours)
	{
		std::array<uint8_t, 4> color = { 0, 0, 0, 0 };
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

	TerrainMeshData GenerateTerrainMeshFromCells(const Array2D<TerrainCell>& chunk_cells)
	{
		TerrainMeshData data;

		// Distribute terrain types to textures slots
		const auto& [slot_map, slots] = AllocateTerrainTextureSlots(chunk_cells);

		// Create vertex and index buffers
		const auto chunk_cells_count = TerrainChunk::Size * TerrainChunk::Size;

		data.vertices.reserve(std::size(CellGeometry::vertices) * chunk_cells_count);
		data.indices.reserve(std::size(CellGeometry::indices) * chunk_cells_count);
		data.tex_slots = slots;

		for (Point cell_pos : utils::rect_points_rt2lb(TerrainChunk::Area))
		{
			// Append indices and vertices
			EmitIndices(data.vertices.size(), data.indices);
			auto cell_verts = EmitCellVertices(data.vertices);

			// Calculate 3D positions of vertices in world
			const auto positions = CalcVertexPositions(cell_pos, chunk_cells);

			// Calculate 3D normals of vertices in world
			const auto normals = CalcVertexNormals(cell_pos, chunk_cells);

			WriteVertexPositions(cell_verts, positions);
			WriteVertexUVs(cell_verts, 0.5f, positions);
			WriteVertexNormals(cell_verts, normals);
			WriteVertexColors(cell_verts, cell_pos, slot_map);
		}

		return data;
	}


	Array2D<TerrainCell> GetExtendedChunkCells(World& world, Point chunk_pos)
	{
		Array2D<TerrainCell> cells{ Inflated(TerrainChunk::Area, 1, 1) };

		auto* map = world.globals.Get<ChunkMap>();
		if (!map || !map->chunks.IndexIsValid(chunk_pos))
			return cells;

		// Fill central part
		const auto chunk_ent = map->chunks[chunk_pos];
		if (chunk_ent)
		{
			if (auto* chunk = world.entities.GetComponent<TerrainChunk>(chunk_ent)) {
				CopyArrayData(chunk->cells, cells, chunk->cells.GetRect(), LeftBottom(chunk->cells.GetRect()));
			}
		}

		// Fill edges
		static constexpr Point Offsets[] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}};
		for (const Point offset : Offsets)
		{
			const Point nchunk_pos = chunk_pos + offset;

			const auto dst_area = Intersection(cells.GetRect(), TerrainChunk::Area + offset * TerrainChunk::Size);
			const auto src_area = dst_area - offset * TerrainChunk::Size;

			const auto nchunk_ent = map->chunks.GetOrDef(nchunk_pos, ecs::Entity{});
			if (nchunk_ent)
			{
				if (auto* chunk = world.entities.GetComponent<TerrainChunk>(nchunk_ent)) {
					CopyArrayData(chunk->cells, cells, src_area, LeftBottom(dst_area));
				}
			}
			else
			{
				// TODO: try to duplicate data from central chunk
			}
		}

		return cells;
	}

	std::future<TerrainMeshData> GenerateTerrainMesh(World& world, Point chunk_pos)
	{
		const auto chunk_cells_ex = GetExtendedChunkCells(world, chunk_pos);
		return utils::Async(GenerateTerrainMeshFromCells, chunk_cells_ex);
	}
}