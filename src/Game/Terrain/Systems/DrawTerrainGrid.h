#pragma once

#include "Game/ISystem.h"
#include "Render/IRenderer.h"

namespace Expanse::Game::Terrain
{
	class RenderGrid : public ISystem
	{
	public:
		RenderGrid(World& w, Render::IRenderer* r);

		void Update() override;

	private:
		Render::IRenderer* renderer = nullptr;
		Render::Material material;

		void UpdateGeometry();
		void Draw();
	};
}