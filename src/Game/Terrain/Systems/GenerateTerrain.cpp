#include "GenerateTerrain.h"

#include "Game/World.h"
#include "Game/Terrain/TerrainData.h"

#include "Utils/Random.h"

namespace Expanse::Game::Terrain
{
	void GenerateCells::Update()
	{
		if (!world.entities.HasComponents<TerrainChunk>())
		{
			static constexpr Point Chunks[] = {
				{0, 0}, {-1, 0}, {-1, -1}, {0, -1}
			};

			static constexpr TerrainType TerrainTypes[] = { TerrainType::Dirt, TerrainType::Grass };

			for (const auto pt : Chunks)
			{
				auto ent = world.entities.CreateEntity();
				auto* chunk = world.entities.AddComponent<TerrainChunk>(ent, pt);

				for (auto& cell : chunk->cells)
				{
					cell.type = TerrainTypes[RandomInt(0,1)];
				}
			}	
		}
	}
}