#pragma once

#include "Game/ISystem.h"

#include "Render/IRenderer.h"

#include "ECS/Entity.h"

#include "Game/Terrain/TerrainData.h"

namespace Expanse::Game::Terrain
{
	class RenderCells : public ISystem
	{
	public:
		RenderCells(World& w, Render::IRenderer* r);

		void Update() override;

	private:
		Render::IRenderer* renderer = nullptr;
		Render::Material terrain_material;

		void GenerateChunksRenderData();
		void GenerateChunkRenderData(ecs::Entity ent);

		void RenderChunks();
	};
}