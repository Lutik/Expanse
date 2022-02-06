#pragma once

#include "Render/VertexTypes.h"
#include "Render/BufferData.h"
#include "Render/RenderTypes.h"

namespace Expanse::Render::GL
{
	class VertexArrayManager
	{
	public:
		VertexArrayManager() = default;
		~VertexArrayManager();
		VertexArrayManager(const VertexArrayManager&) = delete;
		VertexArrayManager& operator=(const VertexArrayManager&) = delete;

		Mesh Create();
		void Free(Mesh handle);
		void SetVertices(Mesh mesh, BufferData vertex_data, const VertexLayout& format);
		void SetIndices(Mesh mesh, BufferData indices_data, size_t index_size);
		void SetPrimitiveType(Mesh mesh, PrimitiveType prim);

		void Draw(Mesh mesh);
		void DrawVertexRange(Mesh mesh, int start_vertex, int vertex_count);
		void DrawIndexRange(Mesh mesh, int start_index, int index_count, int base_vertex);

	private:
		struct VertexArray
		{
			GLuint vao = 0;

			GLuint vbo = 0;	
			int vertex_count = 0;

			GLuint ibo = 0;
			int index_count = 0;
			GLenum index_type = GL_UNSIGNED_SHORT;
			int index_size = 2;

			GLenum prim_type = GL_TRIANGLES;

			void Create();
			void Free();
			void SetVertices(BufferData vertex_data, const VertexLayout& format);
			void SetIndices(BufferData indices_data, size_t index_size);
			void SetPrimitiveType(PrimitiveType prim);
		};

		VertexArray* Bind(Mesh mesh);

		std::vector<VertexArray> vertex_arrays;

		int last_index_size = 0;
	};
}