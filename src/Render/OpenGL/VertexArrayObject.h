#pragma once

#include "Render/VertexTypes.h"
#include "Render/ResourceHandles.h"
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

	private:
		struct VertexArray
		{
			GLuint vao = 0;

			GLuint vbo = 0;	
			GLsizei vertex_count = 0;

			GLuint ibo = 0;
			GLsizei index_count = 0;
			GLenum index_type = GL_UNSIGNED_SHORT;

			GLenum prim_type = GL_TRIANGLES;

			void Create();
			void Free();
			void SetVertices(BufferData vertex_data, const VertexLayout& format);
			void SetIndices(BufferData indices_data, size_t index_size);
			void SetPrimitiveType(PrimitiveType prim);
			void Draw();
		};

		std::vector<VertexArray> vertex_arrays;
	};
}