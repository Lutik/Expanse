#pragma once

#include "Render/VertexAttributes.h"

namespace Expanse::Render
{
	class VertexArray
	{
	public:
		VertexArray() = default;
		~VertexArray();

		VertexArray(const VertexArray& other) = delete;
		VertexArray& operator=(const VertexArray& other) = delete;
		VertexArray(VertexArray&& other);
		VertexArray& operator=(VertexArray&& other);

		template<typename Vertex> //requires(VertexFormat<Vertex>)
		void SetVertices(const std::vector<Vertex>& vertices)
		{
			SetVertices(vertices.data(), vertices.size(), sizeof(Vertex), VertexFormat<Vertex>);
		}

		bool IsValid() const { return vao != 0; }

		void Draw();

	private:

		void SetVertices(const void* vertices_data, size_t vertices_count, size_t vertex_size, const std::vector<VertexAttribDesc>& format);

	private:
		GLuint vao = 0;
		GLuint vbo = 0;
		GLsizei vertices = 0;
	};
}