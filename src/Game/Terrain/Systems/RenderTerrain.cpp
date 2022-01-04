#include "pch.h"

#include "RenderTerrain.h"

#include "Game/World.h"
#include "Game/CoordSystems.h"
#include "Game/Terrain/Components/TerrainMesh.h"
#include "Game/Terrain/Components/TerrainData.h"

namespace Expanse::Game::Terrain
{

	RenderChunks::RenderChunks(World& w, Render::IRenderer* r)
		: ISystem(w)
		, renderer(r)
	{
	}

	void RenderChunks::Update()
	{
		// Gather all chunks
		std::vector<std::pair<Point, TerrainMesh>> chunks;
		world.entities.ForEach<TerrainMesh, TerrainChunk>([&chunks](auto ent, const TerrainMesh& rdata, const TerrainChunk& chunk)
		{
			chunks.emplace_back(chunk.position, rdata);
		});

		// Sort
		auto comp = [](const auto& ch1, const auto& ch2) { return (ch1.first.x + ch1.first.y) > (ch2.first.x + ch2.first.y); };
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