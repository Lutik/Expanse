#pragma once

#include "Render/IRenderer.h"

namespace Expanse::Render::GL
{
	class Renderer : public IRenderer
	{
	public:
		Renderer();

		void ClearFrame() override;

	private:
		void LogOpenGLInfo();
	};
}