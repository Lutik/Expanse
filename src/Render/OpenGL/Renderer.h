#pragma once

#include "Render/IRenderer.h"

#include "Render/OpenGL/VertexArrayObject.h"
#include "Render/OpenGL/MaterialManager.h"

namespace Expanse::Render::GL
{
	class Renderer : public IRenderer
	{
	public:
		Renderer();

		void ClearFrame() override;

		Material CreateMaterial(const std::string& file) override;
		Material CreateMaterial(Material material) override;
		void FreeMaterial(Material material) override;
		void SetMaterialParameter(Material material, std::string_view name, const MaterialParameterValue& value) override;

		Mesh CreateMesh() override;
		void FreeMesh(Mesh mesh) override;
		void SetMeshVertices(Mesh mesh, VertexData data, const VertexLayout& layout) override;

		void Draw(Mesh mesh, Material material) override;

	private:
		void LogOpenGLInfo();

		VertexArrayManager vertex_arrays;
		MaterialManager materials;
	};
}