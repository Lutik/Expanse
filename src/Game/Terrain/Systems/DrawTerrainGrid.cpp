#include "pch.h"

#include "DrawTerrainGrid.h"

#include "Game/World.h"
#include "Game/CoordSystems.h"
#include "Game/Terrain/Components/TerrainData.h"
#include "Game/Terrain/Components/TerrainMesh.h"

#include "Utils/RectPoints.h"

namespace Expanse::Game::Terrain
{
	struct TerrainChunkGrid
	{
		Point chunk_pos;
		Render::Mesh mesh;
	};

	Render::Mesh GenerateGridMesh(Render::IRenderer* renderer, const TerrainChunk& chunk)
	{
		// create array with all grid vertices
		std::vector<Render::VertexP2> vertices;
		vertices.reserve(chunk.cells.heights.Size());

		for (Point cell_pos : utils::rect_points(chunk.cells.heights.GetRect()))
		{
			const auto world_pos = FPoint{ cell_pos };
			const auto height = ToWorldHeight(chunk.cells.heights[cell_pos]);
			vertices.push_back({ Coords::WorldToScene(world_pos, height) });
		}

		// create index array
		std::vector<uint16_t> indices;
		indices.reserve(chunk.cells.heights.Size() * 2);

		auto ToIndex = [w = chunk.cells.heights.Width()](int x, int y){
			return static_cast<uint16_t>(x + y * w);
		};

		for (int x = 0; x < chunk.cells.heights.Width(); ++x)
		{
			indices.push_back(Render::RestartIndex<uint16_t>);
			for (int y = 0; y < chunk.cells.heights.Height(); ++y) {
				indices.push_back(ToIndex(x, y));
			}
		}

		for (int y = 0; y < chunk.cells.heights.Height(); ++y)
		{
			indices.push_back(Render::RestartIndex<uint16_t>);
			for (int x = 0; x < chunk.cells.heights.Width(); ++x) {
				indices.push_back(ToIndex(x, y));
			}
		}

		// upload data to GPU
		return renderer->CreateMesh(vertices, indices, Render::PrimitiveType::LineStrip);
	}

	RenderGrid::RenderGrid(World& w, Render::IRenderer* r)
		: ISystem(w)
		, renderer(r)
	{
		material = renderer->CreateMaterial("content/materials/terrain/grid.json");
	}

	void RenderGrid::UpdateGeometry()
	{
		world.entities.ForEach<TerrainMesh, TerrainChunk>([this](auto ent, const TerrainMesh&, const TerrainChunk& chunk)
		{
			if (!world.entities.HasComponent<TerrainChunkGrid>(ent))
			{
				auto* grid_mesh = world.entities.AddComponent<TerrainChunkGrid>(ent);
				grid_mesh->chunk_pos = chunk.position;
				grid_mesh->mesh = GenerateGridMesh(renderer, chunk);
			}
		});
	}

	void RenderGrid::Draw()
	{
		for (const auto& grid_mesh : world.entities.GetComponentArray<TerrainChunkGrid>())
		{
			const auto world_pos = Coords::LocalToWorld(FPoint{ 0.0f, 0.0f }, grid_mesh.chunk_pos, world.world_origin, TerrainChunk::Size);
			const auto scene_pos = Coords::WorldToScene(world_pos);
			const auto scene_pos_mat = glm::vec2{ scene_pos.x, scene_pos.y };

			renderer->SetMaterialParameter(material, "position", scene_pos_mat);
			renderer->Draw(grid_mesh.mesh, material);
		}
	}

	void RenderGrid::Update()
	{
		UpdateGeometry();
		Draw();
	}
}