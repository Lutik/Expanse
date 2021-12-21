#pragma once

#include "Game/ISystem.h"
#include "Render/IRenderer.h"
#include "Game/Terrain/Components/TerrainData.h"
#include "TerrainMeshGenerator.h"

namespace Expanse::Game::Terrain
{
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
		TerrainMeshGenerator gen;
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
}