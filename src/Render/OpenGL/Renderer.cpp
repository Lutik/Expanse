#include "pch.h"

#include "Render/OpenGL/Renderer.h"

#include "Utils/Logger/Logger.h"

namespace Expanse::Render::GL
{
	static constexpr const char* ViewProjGlobalName = "ViewProjection";

	Renderer::Renderer(Point window_size, Point framebuffer_size)
		: IRenderer(window_size, framebuffer_size)
	{
		glewInit();

		LogOpenGLInfo();

		glEnable(GL_SCISSOR_TEST);

		ResetScissor();
		ResetViewport();

		materials.Init();
		materials.SetGlobalParam(ViewProjGlobalName, &matrices);
	}

	void Renderer::ClearFrame()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer::SetViewport(const Rect& rect)
	{
		glViewport(rect.x, rect.y, rect.w, rect.h);
	}

	void Renderer::SetBgColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void Renderer::SetScissor(const Rect& rect)
	{		
		glScissor(rect.x, rect.y, rect.w, rect.h);
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

	void Renderer::SetMeshVertices(Mesh mesh, BufferData data, const VertexLayout& layout)
	{
		assert(!layout.elements.empty());
		vertex_arrays.SetVertices(mesh, data, layout);
	}

	void Renderer::SetMeshIndices(Mesh mesh, BufferData data, size_t index_size)
	{
		vertex_arrays.SetIndices(mesh, data, index_size);
	}

	void Renderer::SetMeshPrimitiveType(Mesh mesh, PrimitiveType prim_type)
	{
		vertex_arrays.SetPrimitiveType(mesh, prim_type);
	}

	void Renderer::Draw(Mesh mesh, Material material)
	{
		materials.Bind(material);
		vertex_arrays.Draw(mesh);
	}

	void Renderer::DrawVertexRange(Mesh mesh, Material material, int start_vertex, int vertex_count)
	{
		materials.Bind(material);
		vertex_arrays.DrawVertexRange(mesh, start_vertex, vertex_count);
	}
	void Renderer::DrawIndexRange(Mesh mesh, Material material, int start_index, int index_count, int base_vertex)
	{
		materials.Bind(material);
		vertex_arrays.DrawIndexRange(mesh, start_index, index_count, base_vertex);
	}
	
	Texture Renderer::CreateTexture(const std::string& file)
	{
		return materials.CreateTexture(file);
	}

	Texture Renderer::CreateTexture(std::string_view name, const TextureDescription& tex_data)
	{
		return materials.CreateTexture(name, tex_data);
	}

	void Renderer::FreeTexture(Texture texture)
	{
		materials.FreeTexture(texture);
	}

	void Renderer::SetViewProjection(const glm::mat4& view, const glm::mat4& proj)
	{
		matrices.view = view;
		matrices.proj = proj;
		materials.SetGlobalParam(ViewProjGlobalName, &matrices);
	}
}