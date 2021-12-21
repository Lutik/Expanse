#pragma once

#include "Game/Terrain/Components/TerrainMesh.h"
#include "Game/Terrain/Components/TerrainData.h"
#include "Render/IRenderer.h"

namespace Expanse::Game::Terrain
{
	class TerrainMeshGenerator
	{
	public:
		TerrainMeshGenerator(Render::IRenderer* renderer);

		TerrainMesh Generate(const TerrainChunk& chunk) const;

	private:
		Render::IRenderer* renderer = nullptr;
		Render::Material terrain_material;
	};
}