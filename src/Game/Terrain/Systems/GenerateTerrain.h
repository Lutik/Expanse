#pragma once

#include "Game/ISystem.h"

namespace Expanse::Game::Terrain
{
	class GenerateCells : public ISystem
	{
	public:
		GenerateCells(World& w, uint32_t seed);

		void Update();

	private:
		uint32_t types_seed;
	};
}