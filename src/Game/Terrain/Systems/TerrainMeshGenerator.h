#pragma once

#include "Game/Terrain/Components/TerrainMesh.h"
#include "Game/Terrain/Components/TerrainData.h"
#include "Render/VertexTypes.h"

namespace Expanse::Game::Terrain
{
	struct TerrainVertex
	{
		FPoint position;
		FPoint uv;
		glm::vec3 normal;
		Render::Color color;
	};

	static const Render::VertexLayout TerrainVertexFormat = { sizeof(TerrainVertex),
	{
		{ Render::VertexElementUsage::POSITION, sizeof(TerrainVertex::position), offsetof(TerrainVertex, position), sizeof(float), false, false },
		{ Render::VertexElementUsage::TEXCOORD0, sizeof(TerrainVertex::uv), offsetof(TerrainVertex, uv), sizeof(float), false, false },
		{ Render::VertexElementUsage::NORMAL, sizeof(TerrainVertex::normal), offsetof(TerrainVertex, normal), sizeof(float), false, false },
		{ Render::VertexElementUsage::COLOR, sizeof(TerrainVertex::color), offsetof(TerrainVertex, color), sizeof(uint8_t), true, false },
	} };

	using TerrainTextureSlots = std::vector<TerrainType>;

	struct TerrainMeshData
	{
		std::vector<TerrainVertex> vertices;
		std::vector<uint16_t> indices;
		TerrainTextureSlots tex_slots;
	};


	TerrainMeshData GenerateTerrainMesh(const TerrainChunk& chunk);
}