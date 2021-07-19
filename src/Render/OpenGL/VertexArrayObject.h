#pragma once

#include "Render/VertexTypes.h"
#include "Render/ResourceHandles.h"

namespace Expanse::Render::GL
{
	class VertexArrayManager
	{
	public:
		Mesh Create();
		void Free(Mesh handle);
		void SetVertices(Mesh mesh, VertexData vertex_data, const VertexLayout& format);
		void SetIndices(Mesh mesh, VertexData indices_data, size_t index_size);
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

			void Create();
			void Free();
			void SetVertices(VertexData vertex_data, const VertexLayout& format);
			void SetIndices(VertexData indices_data, size_t index_size);
			void Draw();

			bool IsValid() const { return vao != 0; }
		};

		std::vector<VertexArray> vertex_arrays;
	};
}