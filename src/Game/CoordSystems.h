#pragma once

#include "Utils/Math.h"

namespace Expanse::Coords
{
	/*
	* COORDINATE SYSTEMS:
	* 
	* Cell - integer coordinates of individual cells
	* 
	* Chunk - coordinates of chunks, always start at {0,0} in cell coordinates. All chunks cells sizes are equal to standart cells for now.
	* 
	* Local - integer coordinates of cell in chunk
	* Local - floating point coordinates of entity relative to the chunks it is in
	* 
	* World - float coordinates relative to dynamic world origin, world origin is specified in cell coordinates, cell size is 1.0 x 1.0
	* 
	* Scene - float coordinates in screen space, but indepenedent of resolution and scale, 1 unit is roughly 1 cell, {0.0, 0.0} is at world origin
	* 
	* Window - integer coordinates in window pixels. Used for input and/or UI rendering.
	*/

	 
	FRect CellToWorld(Rect cell, Point world_origin);
	FRect CellToWorld(Point cell, Point world_origin);

	Point LocalToCell(Point local_pos, Point chunk, int chunk_size);
	Rect LocalToCell(Rect local_rect, Point chunk, int chunk_size);
	Point CellToLocal(Point cell_pos, Point chunk, int chunk_size);

	FPoint LocalToWorld(FPoint local_pos, Point chunk, Point world_origin, int chunk_size);
	FRect LocalToWorld(FRect local_rect, Point chunk, Point world_origin, int chunk_size);
	FPoint WorldToLocal(FPoint world_pos, Point chunk, Point world_origin, int chunk_size);
	FRect WorldToLocal(FRect world_pos, Point chunk, Point world_origin, int chunk_size);

	FPoint LocalToLocal(FPoint local_pos, Point from_chunk, Point to_chunk, int chunk_size);

	Point WorldToCell(FPoint world_pos, Point world_origin);

	FPoint WorldToScene(FPoint world_pos, float height = 0.0f);

	FPoint SceneToWorld(FPoint scene_pos);

	Point CellToChunk(Point cell, int chunk_size);


	FRect WorldRectSceneBounds(FRect world_rect);

	FRect SceneRectWorldBounds(FRect scene_rect);

	Rect WorldRectCellBounds(FRect world_rect, Point world_origin);

	Rect CellRectChunkBounds(Rect cell_rect, int chunk_size);
}