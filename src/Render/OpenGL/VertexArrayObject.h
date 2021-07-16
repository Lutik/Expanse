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
		void Draw(Mesh mesh);

	private:
		struct VertexArray
		{
			GLuint vbo;
			GLuint vao;
			GLsizei vertex_count;

			void Create();
			void Free();
			void SetVertices(VertexData vertex_data, const VertexLayout& format);
			void Draw();

			bool IsValid() const { return vao != 0; }
		};

		std::vector<VertexArray> vertex_arrays;
	};
}