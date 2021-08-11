#include "TerrainHelper.h"

#include "Game/World.h"
#include "Game/CoordSystems.h"

namespace Expanse::Game::Terrain
{
	void TerrainHelper::Update()
	{
		chunks.clear();

		world.entities.ForEach<TerrainChunk>([this](auto ent, const TerrainChunk& chunk)
		{
			chunks.emplace(chunk.position, ent);
		});
	}

	TerrainCell* TerrainHelper::GetCell(Point cell_pos) const
	{
		const auto chunk_pos = Coords::CellToChunk(cell_pos, TerrainChunk::Size);

		auto itr = chunks.find(chunk_pos);
		if (itr == chunks.end())
			return nullptr;

		auto* chunk = world.entities.GetComponent<TerrainChunk>(itr->second);
		if (!chunk)
			return nullptr;

		const auto local_cell = Coords::CellToLocal(cell_pos, chunk_pos, TerrainChunk::Size);

		return &chunk->cells[local_cell];
	}
}