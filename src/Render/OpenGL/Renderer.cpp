#include "pch.h"

#include "Render/OpenGL/Renderer.h"

#include "Utils/Logger/Logger.h"

namespace Expanse::Render::GL
{
	Renderer::Renderer()
	{
		glewInit();

		LogOpenGLInfo();

		glClearColor(0.0f, 0.6f, 0.4f, 1.0f);
	}

	void Renderer::ClearFrame()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer::LogOpenGLInfo()
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


	Material Renderer::CreateMaterial(const std::string& file)
	{
		const auto shader = shaders.Create(file);
		return { shader.index };
	}

	void Renderer::FreeMaterial(Material material)
	{
		shaders.Free({ material.index });
	}

	Mesh Renderer::CreateMesh()
	{
		return vertex_arrays.Create();
	}

	void Renderer::FreeMesh(Mesh mesh)
	{
		vertex_arrays.Free(mesh);
	}

	void Renderer::SetMeshVertices(Mesh mesh, VertexData data, const VertexLayout& layout)
	{
		vertex_arrays.SetVertices(mesh, data, layout);
	}

	void Renderer::Draw(Mesh mesh, Material material)
	{
		shaders.Use({ material.index });

		vertex_arrays.Draw(mesh);
	}
	
}