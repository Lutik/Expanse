#pragma once

#include "Render/RenderTypes.h"

namespace Expanse::Game::Terrain
{
	struct TerrainMesh
	{
		std::vector<std::pair<Render::Mesh, Render::Material>> layers;
	};
}