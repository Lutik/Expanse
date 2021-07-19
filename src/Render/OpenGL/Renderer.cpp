#include "pch.h"

#include "Render/OpenGL/Renderer.h"

#include "Utils/Logger/Logger.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace Expanse::Render::GL
{
	static constexpr const char* ViewProjGlobalName = "ViewProjection";

	Renderer::Renderer()
	{
		glewInit();

		LogOpenGLInfo();

		glClearColor(0.0f, 0.6f, 0.4f, 1.0f);

		materials.SetGlobalParam(ViewProjGlobalName, &matrices);
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
		return materials.Create(file);
	}

	Material Renderer::CreateMaterial(Material material)
	{
		return materials.Create(material);
	}

	void Renderer::FreeMaterial(Material material)
	{
		materials.Free(material);
	}

	void Renderer::SetMaterialParameter(Material material, std::string_view name, const MaterialParameterValue& value)
	{
		materials.SetParameter(material, name, value);
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

	void Renderer::SetMeshIndices(Mesh mesh, VertexData data, size_t index_size)
	{
		vertex_arrays.SetIndices(mesh, data, index_size);
	}

	void Renderer::Draw(Mesh mesh, Material material)
	{
		materials.Bind(material);
		vertex_arrays.Draw(mesh);
	}
	
	Texture Renderer::CreateTexture(const std::string& file)
	{
		return materials.CreateTexture(file);
	}

	void Renderer::FreeTexture(Texture texture)
	{
		materials.FreeTexture(texture);
	}

	void Renderer::Set2DMode(int width, int height)
	{
		matrices.view = glm::mat4{ 1.0f };

		const auto fwidth = static_cast<float>(width);
		const auto fheight = static_cast<float>(height);
		matrices.proj = glm::orthoLH_NO(0.0f, fwidth, 0.0f, fheight, -1.0f, 1.0f);

		materials.SetGlobalParam(ViewProjGlobalName, &matrices);
	}
}