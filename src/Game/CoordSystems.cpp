#include "CoordSystems.h"

#include <cmath>

namespace Expanse::Coords
{
	static constexpr FPoint Axis[2] = {{ 1.6f, 0.4f }, { -1.0f, 0.75f }};


	FRect CellToWorld(Rect cell, Point world_origin)
	{
		return FRect{ cell - world_origin };
	}

	FRect CellToWorld(Point cell, Point world_origin)
	{
		return CellToWorld(Rect{cell.x, cell.y, 1, 1}, world_origin);
	}

	Point LocalToCell(Point local_pos, Point chunk, int chunk_size)
	{
		return local_pos + chunk * chunk_size;
	}

	Rect LocalToCell(Rect local_rect, Point chunk, int chunk_size)
	{
		return local_rect + chunk * chunk_size;
	}

	Point CellToLocal(Point cell_pos, Point chunk, int chunk_size)
	{
		return cell_pos - chunk * chunk_size;
	}

	Rect CellToLocal(Rect cell_rect, Point chunk, int chunk_size)
	{
		return cell_rect - chunk * chunk_size;
	}

	FPoint LocalToWorld(FPoint local_pos, Point chunk, Point world_origin, int chunk_size)
	{
		return local_pos + FPoint{ chunk * chunk_size - world_origin };
	}

	FRect LocalToWorld(FRect local_rect, Point chunk, Point world_origin, int chunk_size)
	{
		return local_rect + FPoint{ chunk * chunk_size - world_origin };
	}

	FPoint WorldToLocal(FPoint world_pos, Point chunk, Point world_origin, int chunk_size)
	{
		return world_pos - FPoint{ chunk * chunk_size - world_origin };
	}

	FRect WorldToLocal(FRect world_rect, Point chunk, Point world_origin, int chunk_size)
	{
		return world_rect - FPoint{ chunk * chunk_size - world_origin };
	}

	FPoint LocalToLocal(FPoint local_pos, Point from_chunk, Point to_chunk, int chunk_size)
	{
		return local_pos + FPoint{ (to_chunk - from_chunk) * chunk_size };
	}

	Point WorldToCell(FPoint world_pos, Point world_origin)
	{
		const auto x = static_cast<int>(std::floor(world_pos.x));
		const auto y = static_cast<int>(std::floor(world_pos.y));
		return Point{ x, y } + world_origin;
	}

	FPoint WorldToScene(FPoint world_pos)
	{
		return Axis[0] * world_pos.x + Axis[1] * world_pos.y;
	}

	FPoint SceneToWorld(FPoint scene_pos)
	{
		constexpr float AxisDet = Axis[0].x * Axis[1].y - Axis[0].y * Axis[1].x;

		const float x_det = Axis[1].y * scene_pos.x - Axis[1].x * scene_pos.y;
		const float y_det = Axis[0].x * scene_pos.y - Axis[0].y * scene_pos.x;
		return { x_det / AxisDet, y_det / AxisDet };
	}

	Point CellToChunk(Point cell, int chunk_size)
	{
		auto to_chunk = [sz = chunk_size](int v) {
			return (v >= 0) ? (v / sz) : ((v + 1) / sz - 1);
		};
		return { to_chunk(cell.x), to_chunk(cell.y) };
	}


	FRect WorldRectSceneBounds(FRect world_rect)
	{
		// TODO: optimize
		const FPoint pts[] = {
			WorldToScene(LeftBottom(world_rect)),
			WorldToScene(LeftTop(world_rect)),
			WorldToScene(RightBottom(world_rect)),
			WorldToScene(RightTop(world_rect)),
		};
		return FRect{ pts[1].x, pts[0].y, pts[2].x - pts[1].x, pts[3].y - pts[0].y };
	}

	FRect ChunkSceneBounds(Point world_origin, Point chunk, int chunk_size)
	{
		const FRect chunk_local_rect{ 0.0f, 0.0f, static_cast<float>(chunk_size), static_cast<float>(chunk_size) };
		const FRect chunk_world_rect = LocalToWorld(chunk_local_rect, chunk, world_origin, chunk_size);

		return WorldRectSceneBounds(chunk_world_rect);
	}
}