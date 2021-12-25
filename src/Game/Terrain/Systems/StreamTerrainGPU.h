#pragma once

#include "Game/ISystem.h"
#include "Render/IRenderer.h"
#include "Game/Terrain/Components/TerrainData.h"
#include "Game/Terrain/Components/TerrainMesh.h"
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
		Render::Material terrain_material;

		TerrainMesh UploadTerrainMeshData(const TerrainMeshData& data);
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