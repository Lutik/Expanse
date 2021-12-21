#pragma once

#include "Game/ISystem.h"
#include "Render/IRenderer.h"

namespace Expanse::Game::Terrain
{
	/*
	* Renders all terrain chunks, for which meshes and materials are present
	*/
	class RenderChunks : public ISystem
	{
	public:
		RenderChunks(World& w, Render::IRenderer* r);

		void Update() override;

	private:
		Render::IRenderer* renderer = nullptr;
	};
}