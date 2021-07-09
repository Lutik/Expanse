#include "Render/Renderer.h"

#include <GL/glew.h>

namespace Expanse
{
	void OpenGLRenderer::Init()
	{
		glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
	}

	void OpenGLRenderer::ClearFrame()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}
}