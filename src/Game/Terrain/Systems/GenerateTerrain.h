#pragma once

#include "Game/ISystem.h"

namespace Expanse::Game::Terrain
{
	class GenerateCells : public ISystem
	{
	public:
		GenerateCells(World& w) : ISystem(w) {}

		void Update();
	};
}