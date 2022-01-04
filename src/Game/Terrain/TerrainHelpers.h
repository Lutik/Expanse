#pragma once

#include "Game/World.h"

namespace Expanse::Game::Terrain
{
	void UpdateChunkMap(World& world);

	std::vector<Point> GetNotLoadedChunksInArea(World& world, Rect chunks_area);
}