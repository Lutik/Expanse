#pragma once

#include <memory>

namespace Expanse::Render
{
	struct IRenderer
	{
		virtual ~IRenderer() = default;
		virtual void ClearFrame() = 0;
	};

	std::unique_ptr<IRenderer> CreateOpenGLRenderer();
}