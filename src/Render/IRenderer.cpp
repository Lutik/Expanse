#include "pch.h"

#include "Render/IRenderer.h"

#include "Render/OpenGL/Renderer.h"

namespace Expanse::Render
{
	RendererPtr CreateOpenGLRenderer(Point window_size, Point framebuffer_size)
	{
		return std::make_unique<GL::Renderer>(window_size, framebuffer_size);
	}
}