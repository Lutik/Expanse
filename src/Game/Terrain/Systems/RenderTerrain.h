#pragma once

#include "Game/ISystem.h"

#include "Render/IRenderer.h"

#include "ECS/Entity.h"

#include "Game/Terrain/TerrainData.h"
#include "Game/Terrain/TerrainHelper.h"

namespace Expanse::Game::Terrain
{
	struct TerrainChunkRenderData
	{
		Render::Mesh mesh;
		Render::Material material;
	};

	/*
	* Generates meshes and materials for loaded chunks, that come into view
	*/
	class LoadChunksToGPU : public ISystem
	{
	public:
		LoadChunksToGPU(World& w, Render::IRenderer* r);

		void Update() override;

	private:
		Render::IRenderer* renderer = nullptr;
		Render::Material terrain_material;
		TerrainHelper helper;

		void GenerateChunkRenderData(ecs::Entity ent);
	};

	/*
	* Frees meshes and materials of chunks, that are no longer in view
	*/
	class UnloadChunksFromGPU : public ISystem
	{
	public:
		UnloadChunksFromGPU(World& w, Render::IRenderer* r);

		void Update() override;

	private:
		Render::IRenderer* renderer = nullptr;
	};


	/*
	* Renders all terrain chunks, for which meshes and materials are present
	*/
	class RenderChunks : public ISystem
	{
	public:
		RenderChunks(World& w, Render::IRenderer* r);

		void Update() override;

	private:
		Render::IRenderer *renderer = nullptr;
	};
}