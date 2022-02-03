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
	static constexpr uint16_t QuadIndices[6] = { 0, 1, 2, 2, 1, 3 };
	struct TerrainVertexParams
	{
		FPoint pos;
		std::vector<Point> points; // ofsets of cells contributing to height and normals (1 or 2)
	};

	struct TerrainMaskParams
	{
		uint8_t nmask;
		std::array<FPoint, 4> uv;
	};

	struct TerrainQuadParams
	{
		std::array<TerrainVertexParams, 4> verts;
		std::vector<TerrainMaskParams> blend;
		uint8_t nmask = 0;
	};

	static const TerrainQuadParams FullQuad = {
		.verts = {{
			{ .pos = {0.0f, 0.0f}, .points = {{0, 0}} },
			{ .pos = {0.0f, 1.0f}, .points = {{0, 1}} },
			{ .pos = {1.0f, 0.0f}, .points = {{1, 0}} },
			{ .pos = {1.0f, 1.0f}, .points = {{1, 1}} },
		}},
		.blend = {
			{ .nmask = 0b0000'0000, .uv = {{ {0.25f, 0.25f}, {0.25f, 0.26f}, {0.26f, 0.25f}, {0.26f, 0.26f} }} },
		},
		.nmask = 0
	};
	static const TerrainQuadParams LeftBottomQuad = {
		.verts = {{
			{ .pos = {0.0f, 0.0f}, .points = {{0, 0}        } },
			{ .pos = {0.0f, 0.5f}, .points = {{0, 0}, {0, 1}} },
			{ .pos = {0.5f, 0.0f}, .points = {{0, 0}, {1, 0}} },
			{ .pos = {0.5f, 0.5f}, .points = {{0, 1}, {1, 0}} },
		}},
		.blend = {
			// corner
			{.nmask = 0b0010'0000, .uv = {{ {0.75f, 0.75f}, {0.75f, 1.0f}, {1.0f, 0.75f}, {1.0f, 1.0f} }} },
			// left side
			{.nmask = 0b0010'1000, .uv = {{ {0.75f, 0.25f}, {0.75f, 0.5f}, {1.0f, 0.25f}, {1.0f, 0.5f} }} },
			{.nmask = 0b0000'1000, .uv = {{ {0.75f, 0.25f}, {0.75f, 0.5f}, {1.0f, 0.25f}, {1.0f, 0.5f} }} },
			// bottom side
			{.nmask = 0b0110'0000, .uv = {{ {0.25f, 0.75f}, {0.25f, 1.0f}, {0.5f, 0.75f}, {0.5f, 1.0f} }} },
			{.nmask = 0b0100'0000, .uv = {{ {0.25f, 0.75f}, {0.25f, 1.0f}, {0.5f, 0.75f}, {0.5f, 1.0f} }} },
			// inner corner
			{.nmask = 0b0110'1000, .uv = {{ {0.25f, 0.25f}, {0.5f, 0.25f}, {0.25f, 0.5f}, {0.5f, 0.5f} }} },
			{.nmask = 0b0100'1000, .uv = {{ {0.25f, 0.25f}, {0.5f, 0.25f}, {0.25f, 0.5f}, {0.5f, 0.5f} }} },
		},
		.nmask = 0b0110'1000
	};
	static const TerrainQuadParams RightBottomQuad = {
		.verts = {{
			{.pos = {0.5f, 0.0f}, .points = {{0, 0}, {1, 0}} },
			{.pos = {0.5f, 0.5f}, .points = {{0, 1}, {1, 0}} },
			{.pos = {1.0f, 0.0f}, .points = {{1, 0}        } },
			{.pos = {1.0f, 0.5f}, .points = {{1, 0}, {1, 1}} },
		}},
		.blend = {
			// corner
			{.nmask = 0b1000'0000, .uv = {{ {0.0f, 0.75f}, {0.0f, 1.0f}, {0.25f, 0.75f}, {0.25f, 1.0f} }} },
			// right side
			{.nmask = 0b0001'0000, .uv = {{ {0.0f, 0.25f}, {0.0f, 0.5f}, {0.25f, 0.25f}, {0.25f, 0.5f} }} },
			{.nmask = 0b1001'0000, .uv = {{ {0.0f, 0.25f}, {0.0f, 0.5f}, {0.25f, 0.25f}, {0.25f, 0.5f} }} },
			// bottom side
			{.nmask = 0b1100'0000, .uv = {{ {0.25f, 0.75f}, {0.25f, 1.0f}, {0.5f, 0.75f}, {0.5f, 1.0f} }} },
			{.nmask = 0b0100'0000, .uv = {{ {0.25f, 0.75f}, {0.25f, 1.0f}, {0.5f, 0.75f}, {0.5f, 1.0f} }} },
			// inner corner
			{.nmask = 0b1101'0000, .uv = {{ {0.5f, 0.25f}, {0.5f, 0.5f}, {0.75f, 0.25f}, {0.75f, 0.5f} }} },
			{.nmask = 0b0101'0000, .uv = {{ {0.5f, 0.25f}, {0.5f, 0.5f}, {0.75f, 0.25f}, {0.75f, 0.5f} }} },
		},
		.nmask = 0b1101'0000
	};
	static const TerrainQuadParams LeftTopQuad = {
		.verts = {{
			{.pos = {0.0f, 0.5f}, .points = {{0, 0}, {0, 1}} },
			{.pos = {0.0f, 1.0f}, .points = {{0, 1}        } },
			{.pos = {0.5f, 0.5f}, .points = {{0, 1}, {1, 0}} },
			{.pos = {0.5f, 1.0f}, .points = {{0, 1}, {1, 1}} },
		}},
		.blend = {
			// corner
			{.nmask = 0b0000'0001, .uv = {{ {0.75f, 0.0f}, {0.75f, 0.25f}, {1.0f, 0.0f}, {1.0f, 0.25f} }} },
			// left side
			{.nmask = 0b0000'1001, .uv = {{ {0.75f, 0.5f}, {0.75f, 0.75f}, {1.0f, 0.5f}, {1.0f, 0.75f} }} },
			{.nmask = 0b0000'1000, .uv = {{ {0.75f, 0.5f}, {0.75f, 0.75f}, {1.0f, 0.5f}, {1.0f, 0.75f} }} },
			// top side
			{.nmask = 0b0000'0011, .uv = {{ {0.25f, 0.0f}, {0.25f, 0.25f}, {0.5f, 0.0f}, {0.5f, 0.25f} }} },
			{.nmask = 0b0000'0010, .uv = {{ {0.25f, 0.0f}, {0.25f, 0.25f}, {0.5f, 0.0f}, {0.5f, 0.25f} }} },
			// inner corner
			{.nmask = 0b0000'1011, .uv = {{ {0.25f, 0.5f}, {0.25f, 0.75f}, {0.5f, 0.5f}, {0.5f, 0.75f} }} },
			{.nmask = 0b0000'1010, .uv = {{ {0.25f, 0.5f}, {0.25f, 0.75f}, {0.5f, 0.5f}, {0.5f, 0.75f} }} },
		},
		.nmask = 0b0000'1011
	};
	static const TerrainQuadParams RightTopQuad = {
		.verts = {{
			{.pos = {0.5f, 0.5f}, .points = {{0, 1}, {1, 0}} },
			{.pos = {0.5f, 1.0f}, .points = {{0, 1}, {1, 1}} },
			{.pos = {1.0f, 0.5f}, .points = {{1, 0}, {1, 1}} },
			{.pos = {1.0f, 1.0f}, .points = {{1, 1}        } },
		}},
		.blend = {
			// corner
			{.nmask = 0b0000'0100, .uv = {{ {0.0f, 0.0f}, {0.0f, 0.25f}, {0.25f, 0.0f}, {0.25f, 0.25f} }} },
			// right side
			{.nmask = 0b0001'0100, .uv = {{ {0.0f, 0.5f}, {0.0f, 0.75f}, {0.25f, 0.5f}, {0.25f, 0.75f} }} },
			{.nmask = 0b0001'0000, .uv = {{ {0.0f, 0.5f}, {0.0f, 0.75f}, {0.25f, 0.5f}, {0.25f, 0.75f} }} },
			// top side
			{.nmask = 0b0000'0010, .uv = {{ {0.5f, 0.0f}, {0.5f, 0.25f}, {0.75f, 0.0f}, {0.75f, 0.25f} }} },
			{.nmask = 0b0000'0110, .uv = {{ {0.5f, 0.0f}, {0.5f, 0.25f}, {0.75f, 0.0f}, {0.75f, 0.25f} }} },
			// inner corner
			{.nmask = 0b0001'0110, .uv = {{ {0.5f, 0.5f}, {0.5f, 0.75f}, {0.75f, 0.5f}, {0.75f, 0.75f} }} },
		},
		.nmask = 0b0001'0110
	};

	glm::vec3 CalcSmoothNormal(Point vtx_pos, const Array2D<HeightType>& chunk_heightmap)
	{
		glm::vec3 n;
		n.x = ToWorldHeight(chunk_heightmap[vtx_pos + Offset::Left] - chunk_heightmap[vtx_pos + Offset::Right]);
		n.y = ToWorldHeight(chunk_heightmap[vtx_pos + Offset::Down] - chunk_heightmap[vtx_pos + Offset::Up]);
		n.z = 2.0f;

		return glm::normalize(n);
	}

	glm::vec3 CalcAvgSmoothNormal(Point cell_pos, const std::vector<Point>& offsets, const Array2D<HeightType>& chunk_heightmap)
	{
		glm::vec3 n{0.0f};
		for (const auto off : offsets) {
			n += CalcSmoothNormal(cell_pos + off, chunk_heightmap);
		}
		return glm::normalize(n);
	}

	float CalcAvgHeight(Point cell_pos, const std::vector<Point>& offsets, const Array2D<HeightType>& chunk_heightmap)
	{
		float h = 0.0f;
		for (const auto off : offsets) {
			h += ToWorldHeight(chunk_heightmap[cell_pos + off]);
		}
		return h / static_cast<float>(offsets.size());
	}


	uint8_t CalcNeighboursMask(Point cell_pos, TerrainType type, const Array2D<TerrainType>& chunk_terrain)
	{
		uint8_t mask = 0;
		uint8_t val = 1;
		for (const auto off : Offset::Neighbors8)
		{
			if (chunk_terrain[cell_pos + off] == type) {
				mask = mask | val;
			}
			val = val << 1;
		}
		return mask;
	}


	TerrainVertex GenTerrainVertex(Point cell_pos, const TerrainVertexParams& params, const Array2D<HeightType>& chunk_heightmap)
	{
		TerrainVertex vtx;

		const auto world_pos = FPoint{cell_pos} + params.pos;
		const auto height = CalcAvgHeight(cell_pos, params.points, chunk_heightmap);
		vtx.position = Coords::WorldToScene(world_pos, height);

		vtx.normal = CalcAvgSmoothNormal(cell_pos, params.points, chunk_heightmap);

		vtx.uv = params.pos;

		return vtx;
	}

	void GenerateQuad(const TerrainQuadParams& quad, TerrainTypeMeshData& data, Point cell_pos, uint8_t nmask, const Array2D<HeightType>& chunk_heightmap)
	{
		// Find blend mask
		nmask = nmask & quad.nmask;
		const FPoint* mask_uvs = nullptr;
		for (const auto& mask_param : quad.blend) {
			if (nmask == mask_param.nmask) {
				mask_uvs = mask_param.uv.data();
			}
		}
		if (!mask_uvs) return;

		// Emit indices
		const auto base_idx = static_cast<uint16_t>(data.vertices.size());
		for (const auto idx : QuadIndices) {
			data.indices.push_back(base_idx + idx);
		}

		// Emit vertices
		for (size_t i = 0; i < quad.verts.size(); ++i)
		{
			auto vtx = GenTerrainVertex(cell_pos, quad.verts[i], chunk_heightmap);

			vtx.mask_uv = mask_uvs[i];

			data.vertices.push_back(vtx);
		}
	}

	TerrainTypeMeshData GenerateRenderDataForChunkLayer(TerrainType type, const Array2D<TerrainType>& chunk_terrain, const Array2D<HeightType>& heightmap)
	{
		TerrainTypeMeshData data;
		data.type = type;

		for (Point cell_pos : utils::rect_points_rt2lb(TerrainChunk::Area))
		{
			const auto cell_type = chunk_terrain[cell_pos];
			if (cell_type == type)
			{
				GenerateQuad(FullQuad, data, cell_pos, 0, heightmap);
			}
			else if (cell_type < type)
			{
				const auto nmask = CalcNeighboursMask(cell_pos, type, chunk_terrain);

				GenerateQuad(RightTopQuad, data, cell_pos, nmask, heightmap);
				GenerateQuad(LeftTopQuad, data, cell_pos, nmask, heightmap);
				GenerateQuad(RightBottomQuad, data, cell_pos, nmask, heightmap);
				GenerateQuad(LeftBottomQuad, data, cell_pos, nmask, heightmap);	
			}
		}

		return data;
	}

	TerrainMeshData GenerateTerrainMeshFromCells(const Array2D<TerrainType>& chunk_terrain, const Array2D<HeightType>& chunk_heightmap)
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