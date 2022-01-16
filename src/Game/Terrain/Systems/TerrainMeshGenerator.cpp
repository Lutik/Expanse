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
	static constexpr size_t CellVertexCount = 4;
	static constexpr Point CellVertices[CellVertexCount] = { {0, 0}, {0, 1}, {1, 0}, {1, 1} };
	static constexpr uint16_t CellIndices[6] = { 0, 1, 2, 2, 1, 3 };

	glm::vec3 CalcNormal(Point vtx_pos, const Array2D<float>& heights)
	{
		glm::vec3 n;
		n.x = heights[vtx_pos + Offset::Left] - heights[vtx_pos + Offset::Right];
		n.y = heights[vtx_pos + Offset::Down] - heights[vtx_pos + Offset::Up];
		n.z = 2.0f;

		return glm::normalize(n);
	}

	TerrainVertex GenVertex(Point cell_pos, size_t vtx_index, const Array2D<float>& chunk_heightmap)
	{
		TerrainVertex vtx;

		const Point vtx_pos = cell_pos + CellVertices[vtx_index];

		// isometric position
		const FPoint scene_pos = Coords::WorldToScene(FPoint{vtx_pos});
		vtx.position = scene_pos;

		// add height
		vtx.position.y += chunk_heightmap[vtx_pos];

		// UV
		vtx.uv = FPoint{ CellVertices[vtx_index] };

		// normals
		vtx.normal = CalcNormal(vtx_pos, chunk_heightmap);

		return vtx;
	}

	void GenerateUnblendedCell(TerrainTypeMeshData& data, Point cell_pos, const Array2D<float>& chunk_heightmap)
	{
		// Emit indices
		const auto base_idx = static_cast<uint16_t>(data.vertices.size());
		for (const auto idx : CellIndices) {
			data.indices.push_back(base_idx + idx);
		}

		// Emit vertices
		for (size_t i = 0; i < CellVertexCount; ++i) {
			data.vertices.push_back(GenVertex(cell_pos, i, chunk_heightmap));
		}
	}

	TerrainTypeMeshData GenerateRenderDataForChunkLayer(TerrainType type, const Array2D<TerrainType>& chunk_terrain, const Array2D<float>& heightmap)
	{
		TerrainTypeMeshData data;
		data.type = type;

		for (Point cell_pos : utils::rect_points_rt2lb(TerrainChunk::Area))
		{
			const auto cell_type = chunk_terrain[cell_pos];
			if (cell_type == type)
			{
				GenerateUnblendedCell(data, cell_pos, heightmap);
			}
			else if (cell_type < type)
			{
				// generate blend
			}
		}

		return data;
	}

	TerrainMeshData GenerateTerrainMeshFromCells(const Array2D<TerrainType>& chunk_terrain, const Array2D<float>& chunk_heightmap)
	{
		TerrainMeshData data;

		for (TerrainType type = 0; type < 3; ++type)
		{
			auto layer_data = GenerateRenderDataForChunkLayer(type, chunk_terrain, chunk_heightmap);
			if (!layer_data.vertices.empty()) {
				data.layers.push_back(std::move(layer_data));
			}
		}

		return data;
	}


	TerrainCellsArray GetExtendedChunkCells(World& world, Point chunk_pos)
	{
		TerrainCellsArray cells{ Inflated(TerrainChunk::Area, 1, 1) };

		auto* map = world.globals.Get<ChunkMap>();
		if (!map || !map->chunks.IndexIsValid(chunk_pos))
			return cells;

		// Fill central part
		const auto chunk_ent = map->chunks[chunk_pos];
		if (chunk_ent)
		{
			if (auto* chunk = world.entities.GetComponent<TerrainChunk>(chunk_ent)) {
				CopyArrayData(chunk->cells.types, cells.types, TerrainChunk::Area, LeftBottom(TerrainChunk::Area));
				CopyArrayData(chunk->cells.heights, cells.heights, TerrainChunk::AreaVtx, LeftBottom(TerrainChunk::AreaVtx));
			}
		}

		// Fill edges
		static constexpr Point Offsets[] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}};
		for (const Point offset : Offsets)
		{
			const Point nchunk_pos = chunk_pos + offset;

			const auto dst_area_cells = Intersection(cells.types.GetRect(), TerrainChunk::Area + offset * TerrainChunk::Size);
			const auto src_area_cells = dst_area_cells - offset * TerrainChunk::Size;

			const auto dst_area_vtx = Intersection(cells.heights.GetRect(), TerrainChunk::AreaVtx + offset * TerrainChunk::Size);
			const auto src_area_vtx = dst_area_vtx - offset * TerrainChunk::Size;

			const auto nchunk_ent = map->chunks.GetOrDef(nchunk_pos, ecs::Entity{});
			if (nchunk_ent)
			{
				if (auto* chunk = world.entities.GetComponent<TerrainChunk>(nchunk_ent)) {
					CopyArrayData(chunk->cells.types, cells.types, src_area_cells, LeftBottom(dst_area_cells));
					CopyArrayData(chunk->cells.heights, cells.heights, src_area_vtx, LeftBottom(dst_area_vtx));
				}
			}
		}

		return cells;
	}

	std::future<TerrainMeshData> GenerateTerrainMesh(World& world, Point chunk_pos)
	{
		const auto chunk_cells_ex = GetExtendedChunkCells(world, chunk_pos);
		return utils::Async(GenerateTerrainMeshFromCells, chunk_cells_ex.types, chunk_cells_ex.heights);
	}
}