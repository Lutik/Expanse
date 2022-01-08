#pragma once

#include "Game/ISystem.h"
#include "Game/Terrain/Components/TerrainData.h"
#include "Game/Terrain/Systems/TerrainLoader.h"
#include "Utils/Math.h"

namespace Expanse::Game::Terrain
{
	class LoadChunks : public ISystem
	{
	public:
		LoadChunks(World& w, uint32_t seed, Point window_size);

		void Update() override;

		template<class T, typename... Args> requires std::is_base_of_v<ITerrainLoader, T>
		void AddLoader(Args&&... args) {
			loaders.push_back(std::make_unique<T>(std::forward<Args>(args)...));
		}
	private:
		Point window_size;
		Rect loaded_area{ 0, 0, 0, 0 };

		std::vector<std::unique_ptr<ITerrainLoader>> loaders;

		ITerrainLoader* GetLoaderForChunk(Point chunk_pos);
	};

	class UnloadChunks : public ISystem
	{
	public:
		UnloadChunks(World& w, Point window_size);

		void Update() override;

	private:
		Point window_size;
	};
}