#include "CoordSystems.h"

#include <cmath>

namespace Expanse::Coords
{
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
		return chunk * chunk_size + local_pos;
	}

	FPoint LocalToWorld(FPoint local_pos, Point chunk, Point world_origin, int chunk_size)
	{
		return FPoint{ chunk * chunk_size - world_origin } + local_pos;
	}

	FPoint WorldToLocal(FPoint world_pos, Point chunk, Point world_origin, int chunk_size)
	{
		return world_pos - FPoint{ chunk * chunk_size - world_origin };
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
		return { world_pos.x + world_pos.y, 0.5f * (world_pos.y - world_pos.x) };
	}

	FPoint SceneToWorld(FPoint scene_pos)
	{
		return { 0.5f * scene_pos.x - scene_pos.y, 0.5f * scene_pos.x + scene_pos.y };
	}

	Point CellToChunk(Point cell, int chunk_size)
	{
		auto to_chunk = [sz = chunk_size](int v) {
			return (v >= 0) ? (v / sz) : ((v + 1) / sz - 1);
		};
		return { to_chunk(cell.x), to_chunk(cell.y) };
	}
}