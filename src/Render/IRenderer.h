#pragma once

#include <memory>
#include <string>

#include "ResourceHandles.h"
#include "VertexTypes.h"

namespace Expanse::Render
{
	struct IRenderer
	{
		virtual ~IRenderer() = default;

		virtual void ClearFrame() = 0;

		virtual Material CreateMaterial(const std::string& file) = 0;
		virtual void FreeMaterial(Material material) = 0;

		virtual Mesh CreateMesh() = 0;
		virtual void FreeMesh(Mesh mesh) = 0;
		virtual void SetMeshVertices(Mesh mesh, VertexData data, const VertexLayout& layout) = 0;

		template<class Vertex>
		void SetMeshVertices(Mesh mesh, const std::vector<Vertex>& vertices) {
			SetMeshVertices(mesh, VertexData{ vertices }, VertexFormat<Vertex>);
		}

		virtual void Draw(Mesh mesh, Material material) = 0;
	};

	std::unique_ptr<IRenderer> CreateOpenGLRenderer();
}