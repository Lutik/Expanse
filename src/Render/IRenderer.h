#pragma once

#include <memory>
#include <string>

#include "VertexTypes.h"
#include "BufferData.h"
#include "RenderTypes.h"
#include "Common/ResourceDescriptions.h"

#include "glm/ext/matrix_clip_space.hpp"

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

		// Sets viewport rectangle
		virtual void SetViewport(const Rect& rect) = 0;

		// Sets background color, used when clearing framebuffer
		virtual void SetBgColor(const glm::vec4& color) = 0;

		// Sets scissor rect
		virtual void SetScissor(const Rect& rect) = 0;

		// Resets viewport to include whole window
		void ResetViewport() {
			SetViewport(GetFramebufferRect());
		}

		// Disables scissor rect
		void ResetScissor() {
			SetScissor(GetFramebufferRect());
		}

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

		template<std::ranges::contiguous_range VertexRange>
		void SetMeshVertices(Mesh mesh, const VertexRange& vertices)
		{
			SetMeshVertices(mesh, vertices, VertexFormat<std::ranges::range_value_t<VertexRange>>);
		}

		template<std::ranges::contiguous_range IndexRange>
		void SetMeshIndices(Mesh mesh, const IndexRange& indices)
		{
			SetMeshIndices(mesh, indices, sizeof(std::ranges::range_value_t<IndexRange>));
		}

		Mesh CreateMesh(PrimitiveType prim_type)
		{
			auto mesh = CreateMesh();
			SetMeshPrimitiveType(mesh, prim_type);
			return mesh;
		}

		template<std::ranges::contiguous_range VertexRange>
		Mesh CreateMesh(const VertexRange& vertices, PrimitiveType prim_type = PrimitiveType::Triangles)
		{
			auto mesh = CreateMesh(prim_type);
			SetMeshVertices(mesh, vertices);
			return mesh;
		}

		template<std::ranges::contiguous_range VertexRange, std::ranges::contiguous_range IndexRange>
		Mesh CreateMesh(const VertexRange& vertices, const IndexRange& indices, PrimitiveType prim_type = PrimitiveType::Triangles)
		{
			auto mesh = CreateMesh(prim_type);
			SetMeshVertices(mesh, vertices);
			SetMeshIndices(mesh, indices);
			return mesh;
		}

		/***********************************************************************************/

		// Clear framebuffer
		virtual void ClearFrame() = 0;

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

		// Sets view and projection matrices (for those materials that use them)
		virtual void SetViewProjection(const glm::mat4& view, const glm::mat4& proj) = 0;

		// Sets matrices to use window coordinate system
		void Set2DMode(const FRect& rect)
		{
			const auto view = glm::mat4{ 1.0f };
			const auto proj = glm::orthoLH_NO(rect.x, rect.x + rect.w, rect.y, rect.y + rect.h, -1.0f, 1.0f);
			SetViewProjection(view, proj);
		}

		/***********************************************************************************/

		// Returns window size
		Point GetWindowSize() const { return window_size; }
		Rect GetWindowRect() const { return Rect{0, 0, window_size.x, window_size.y }; }

		// Returns framebuffer size (may be not equal to window size on retina devices)
		Point GetFramebufferSize() const { return framebuffer_size; }
		Rect GetFramebufferRect() const { return Rect{0, 0, framebuffer_size.x, framebuffer_size.y }; }

	protected:
		Point window_size;
		Point framebuffer_size;
	};

	using RendererPtr = std::unique_ptr<IRenderer>;

	RendererPtr CreateOpenGLRenderer(Point window_size, Point framebuffer_size);

	template<typename IndexType>
	inline constexpr auto RestartIndex = std::numeric_limits<IndexType>::max();
}