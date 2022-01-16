#pragma once

#include "Game/Terrain/Components/TerrainMesh.h"
#include "Game/Terrain/Components/TerrainData.h"
#include "Render/VertexTypes.h"
#include "Game/World.h"

namespace Expanse::Game::Terrain
{
	struct TerrainVertex
	{
		FPoint position;
		FPoint uv;
		glm::vec3 normal;
	};

	static const Render::VertexLayout TerrainVertexFormat = { sizeof(TerrainVertex),
	{
		{ Render::VertexElementUsage::POSITION, sizeof(TerrainVertex::position), offsetof(TerrainVertex, position), sizeof(float), false, false },
		{ Render::VertexElementUsage::TEXCOORD0, sizeof(TerrainVertex::uv), offsetof(TerrainVertex, uv), sizeof(float), false, false },
		{ Render::VertexElementUsage::NORMAL, sizeof(TerrainVertex::normal), offsetof(TerrainVertex, normal), sizeof(float), false, false },
	} };

	struct TerrainTypeMeshData
	{
		TerrainType type;
		std::vector<TerrainVertex> vertices;
		std::vector<uint16_t> indices;
	};

	struct TerrainMeshData
	{
		std::vector<TerrainTypeMeshData> layers;
	};

	std::future<TerrainMeshData> GenerateTerrainMesh(World& world, Point chunk_pos);
}