#pragma once

#include <memory>
#include <string>

#include "VertexTypes.h"
#include "BufferData.h"
#include "RenderTypes.h"
#include "Common/ResourceDescriptions.h"

namespace Expanse::Render
{
	class IRenderer
	{
		static constexpr int MaxElemCount = std::numeric_limits<int>::max();
	public:
		IRenderer(Point wnd_size, Point fb_size)
			: window_size(wnd_size)
			, framebuffer_size(fb_size)
		{}

		virtual ~IRenderer() = default;

		/***********************************************************************************/
		virtual void ClearFrame() = 0;

		virtual void SetViewport(const Rect& rect) = 0;

		/***********************************************************************************/

		// Create material from its description in a file
		virtual Material CreateMaterial(const std::string& file) = 0;

		// Create material by duplicating already existing material
		virtual Material CreateMaterial(Material material) = 0;

		// Destroy material and free resources used by it (shaders, textures), if they are not used anywhere else
		virtual void FreeMaterial(Material material) = 0;	

		// Set material parameters (uniforms or textures)
		virtual void SetMaterialParameter(Material material, std::string_view name, const MaterialParameterValue& value) = 0;

		/***********************************************************************************/

		// Create an empty mesh
		virtual Mesh CreateMesh() = 0;

		// Destroy mesh
		virtual void FreeMesh(Mesh mesh) = 0;

		// Set mesh data and paramaters
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

		// Draws whole mesh
		virtual void Draw(Mesh mesh, Material material) = 0;

		// Draws selected range of vertices from the mesh
		virtual void DrawVertexRange(Mesh mesh, Material material, int start_vertex, int vertex_count = MaxElemCount) = 0;

		// Draws selected range of indices from the mesh. Undefined if used with unindexed mesh.
		virtual void DrawIndexRange(Mesh mesh, Material material, int start_index, int count = MaxElemCount, int base_vertex = 0) = 0;

		/***********************************************************************************/

		// Create texture by loading it from file
		virtual Texture CreateTexture(const std::string& file) = 0;

		// Create texture by loading it from memory. Overwrites texture, if the same name already exists
		virtual Texture CreateTexture(std::string_view name, const TextureDescription& tex_data) = 0;

		// Destroy texture (or rather decrement its ref count)
		virtual void FreeTexture(Texture texture) = 0;

		/***********************************************************************************/
		virtual void SetViewProjection(const glm::mat4& view, const glm::mat4& proj) = 0;

		virtual void Set2DMode() = 0;

		Point GetWindowSize() const { return window_size; }
		Point GetFramebufferSize() const { return framebuffer_size; }

	protected:
		Point window_size;
		Point framebuffer_size;
	};

	using RendererPtr = std::unique_ptr<IRenderer>;

	RendererPtr CreateOpenGLRenderer(Point window_size, Point framebuffer_size);
}