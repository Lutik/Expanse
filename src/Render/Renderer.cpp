#include "pch.h"

#include "Render/Renderer.h"
#include "Utils/Logger/Logger.h"

namespace Expanse
{
	OpenGLRenderer::OpenGLRenderer()
	{
		glewInit();

		LogOpenGLInfo();

		glClearColor(0.0f, 0.6f, 0.4f, 1.0f);
	}

	void OpenGLRenderer::ClearFrame()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderer::LogOpenGLInfo()
	{
		const auto vendor = (const char*)glGetString(GL_VENDOR);
		const auto renderer = (const char*)glGetString(GL_RENDERER);
		const auto gl_version = (const char*)glGetString(GL_VERSION);
		const auto glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

		Log::message("Vendor: {}", vendor);
		Log::message("Renderer: {}", renderer);
		Log::message("GL version: {}", gl_version);
		Log::message("GLSL version: {}", glsl_version);
	}
}