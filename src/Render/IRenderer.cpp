#include "pch.h"

#include "Render/IRenderer.h"

#include "Render/OpenGL/Renderer.h"

namespace Expanse::Render
{
	RendererPtr CreateOpenGLRenderer()
	{
		return std::make_unique<GL::Renderer>();
	}
}