#pragma once

#include "Render/IRenderer.h"

#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/VertexArrayObject.h"

namespace Expanse::Render::GL
{
	class Renderer : public IRenderer
	{
	public:
		Renderer();

		void ClearFrame() override;

		Material CreateMaterial(const std::string& file) override;
		void FreeMaterial(Material material) override;

		Mesh CreateMesh() override;
		void FreeMesh(Mesh mesh) override;
		void SetMeshVertices(Mesh mesh, VertexData data, const VertexLayout& layout) override;

		void Draw(Mesh mesh, Material material) override;

	private:
		void LogOpenGLInfo();

		ShaderProgramsManager shaders;
		VertexArrayManager vertex_arrays;
	};
}