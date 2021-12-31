#pragma once

#include "Game/ISystem.h"
#include "Game/World.h"
#include "Render/IRenderer.h"

namespace Expanse
{
	enum class AppState
	{
		None,
		MainMenu,
		Game,
		Quit
	};

	class IAppState
	{
	public:
		virtual ~IAppState() = default;

		virtual void Update(float dt) = 0;

		void SwitchTo(AppState state) { req_state = state; }
		AppState GetRequestedState() const { return req_state; }

	private:
		AppState req_state = AppState::None;
	};
}