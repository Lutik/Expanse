#include "GenerateTerrain.h"

#include "Game/World.h"
#include "Game/CoordSystems.h"

#include "Utils/Random.h"
#include "Utils/RectPoints.h"
#include "Utils/Utils.h"

#include <set>

namespace Expanse::Game::Terrain
{
	namespace
	{
		struct PointCmp
		{
			constexpr bool operator()(const Point& p1, const Point& p2) const
			{
				return (p1.x < p2.x) || ((p1.x == p2.x) && (p1.y < p2.y));
			}
		};

		Rect GetMapAreaToLoad(World& world, Point window_size)
		{
			const auto window_rect = FRect{ 0, 0, static_cast<float>(window_size.x), static_cast<float>(window_size.y) };
			const auto view_rect = Centralized(window_rect) / world.camera_scale + world.camera_pos;
			ScaleFromCenter(view_rect, 2.0f);

			const auto world_rect = Coords::SceneRectWorldBounds(view_rect);
			const auto cell_rect = Coords::WorldRectCellBounds(world_rect, world.world_origin);
			const auto chunks_area = Coords::CellRectChunkBounds(cell_rect, TerrainChunk::Size);
			return chunks_area;
		}

		std::vector<Point> GetChunksToLoad(World& world, Point window_size)
		{
			auto get_pos = [](const TerrainChunk& chunk){ return chunk.position; };
			const auto loaded_view = world.entities.GetComponentArray<TerrainChunk>() | std::views::transform(get_pos);
			std::set<Point, PointCmp> loaded_chunks{ loaded_view.begin(), loaded_view.end() };

			const auto req_area = utils::rect_points{ GetMapAreaToLoad(world, window_size) };
			std::set<Point, PointCmp> req_chunks{ req_area.begin(), req_area.end() };

			std::vector<Point> result;
			std::set_difference(req_chunks.begin(), req_chunks.end(), loaded_chunks.begin(), loaded_chunks.end(), std::back_inserter(result), PointCmp{});
			return result;
		}
	}




	GenerateChunks::GenerateChunks(World& w, uint32_t seed, Point wnd_size)
		: ISystem(w)
		, window_size(wnd_size)
	{
		types_seed = Squirrel3(0, seed);
		heights_seed[0] = Squirrel3(1, seed);
		heights_seed[1] = Squirrel3(2, seed);
	}

	struct NoiseParam
	{
		float freq;
		float min;
		float max;
	};

	void GenerateChunks::Update()
	{
		const auto chunks_to_load = GetChunksToLoad(world, window_size);
		for (const auto chunk_pos : chunks_to_load)
		{
			auto ent = world.entities.CreateEntity();
			auto* chunk = world.entities.AddComponent<TerrainChunk>(ent, chunk_pos);

			LoadChunk(*chunk);
		}
	}

	void GenerateChunks::LoadChunk(TerrainChunk& chunk)
	{
		for (const auto local_pos : utils::rect_points{ chunk.cells.GetRect() })
		{
			const auto cell_pos = Coords::LocalToCell(local_pos, chunk.position, TerrainChunk::Size);

			auto& cell = chunk.cells[local_pos];

			cell.type = GetTerrainAt(cell_pos);
			cell.height = static_cast<int>(GetHeightAt(cell_pos));
		}
	}

	float GenerateChunks::GetHeightAt(Point cell_pos) const
	{
		static constexpr NoiseParam heights_noise_params[] = {
			{ 0.05f, -20.0f, 20.0f },
			{ 0.27f, -5.0f, 5.0f },
		};

		float height = 0.0f;
		utils::for_each_zipped(heights_seed, heights_noise_params, [&height, cell_pos](uint32_t seed, const auto& params)
		{
			const auto pt = FPoint{ cell_pos } * params.freq;
			height += Lerp(params.min, params.max, PerlinNoise(pt, seed));
		});

		return height;
	}

	TerrainType GenerateChunks::GetTerrainAt(Point cell_pos) const
	{
		const auto pt = FPoint{ cell_pos } * 0.07f;
		const float n = PerlinNoise(pt, types_seed);

		return (n > 0.0f) ? TerrainType::Grass : TerrainType::Dirt;
	}

	/*************************************************************************************************/

	UnloadChunks::UnloadChunks(World& w, Point wnd_size)
		: ISystem(w)
		, window_size(wnd_size)
	{}

	void UnloadChunks::Update()
	{
		const auto req_area = GetMapAreaToLoad(world, window_size);

		std::vector<ecs::Entity> free_chunks;
		world.entities.ForEach<TerrainChunk>([req_area, &free_chunks](auto ent, const TerrainChunk& chunk)
		{
			if (chunk.use_count <= 0 && !Contains(req_area, chunk.position)) {
				free_chunks.push_back(ent);
			}
		});

		world.entities.DestroyEntities(free_chunks);
	}
}