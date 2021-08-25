#pragma once

#include "Game/ISystem.h"

namespace Expanse::Game::Player
{
	class ScrollCamera : public ISystem
	{
	public:
		explicit ScrollCamera(World& w);

		void Update() override;

	private:
		float camera_speed = 512.0f; // pixels per second

		void UpdateMovement();
		void UpdateZoom();
	};
}