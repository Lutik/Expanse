#pragma once

#include "Game/ISystem.h"

namespace Expanse
{
	class DebugWindowSystem final : public Game::ISystem
	{
	public:
		explicit DebugWindowSystem(Game::World& w)
			: Game::ISystem(w)
		{}

		void Update() override;
	};
}