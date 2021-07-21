#pragma once

#include <memory>
#include <string>

#include "ResourceHandles.h"
#include "VertexTypes.h"
#include "BufferData.h"
#include "MaterialParameters.h"
#include "RenderTypes.h"

namespace Expanse::Render
{
	struct IRenderer
	{
		virtual ~IRenderer() = default;

		/***********************************************************************************/
		virtual void ClearFrame() = 0;

		/***********************************************************************************/
		virtual Material CreateMaterial(const std::string& file) = 0;
		virtual Material CreateMaterial(Material material) = 0;
		virtual void FreeMaterial(Material material) = 0;	

		virtual void SetMaterialParameter(Material material, std::string_view name, const MaterialParameterValue& value) = 0;

		/***********************************************************************************/
		virtual Mesh CreateMesh() = 0;
		virtual void FreeMesh(Mesh mesh) = 0;
		virtual void SetMeshVertices(Mesh mesh, BufferData data, const VertexLayout& layout) = 0;
		virtual void SetMeshIndices(Mesh mesh, BufferData data, size_t index_size) = 0;
		virtual void SetMeshPrimitiveType(Mesh mesh, PrimitiveType prim_type) = 0;

		template<class Vertex>
		void SetMeshVertices(Mesh mesh, const std::vector<Vertex>& vertices) {
			SetMeshVertices(mesh, vertices, VertexFormat<Vertex>);
		}
		template<class Index>
		void SetMeshIndices(Mesh mesh, const std::vector<Index>& indices) {
			SetMeshIndices(mesh, indices, sizeof(Index));
		}
		template<class Vertex, class Index>
		Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, PrimitiveType prim_type = PrimitiveType::Triangles) {
			auto mesh = CreateMesh();
			SetMeshVertices(mesh, vertices);
			SetMeshIndices(mesh, indices);
			SetMeshPrimitiveType(mesh, prim_type);
			return mesh;
		}

		/***********************************************************************************/
		virtual void Draw(Mesh mesh, Material material) = 0;

		/***********************************************************************************/
		virtual Texture CreateTexture(const std::string& file) = 0;
		virtual void FreeTexture(Texture texture) = 0;

		/***********************************************************************************/
		virtual void Set2DMode(int width, int height) = 0;
	};

	std::unique_ptr<IRenderer> CreateOpenGLRenderer();
}