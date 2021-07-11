#include "pch.h"

#include "VertexArrayObject.h"

namespace Expanse::Render::GL
{
	VertexArray::~VertexArray()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

	VertexArray::VertexArray(VertexArray&& other)
	{
		std::swap(vao, other.vao);
		std::swap(vbo, other.vbo);
	}

	VertexArray& VertexArray::operator=(VertexArray&& other)
	{
		std::swap(vao, other.vao);
		std::swap(vbo, other.vbo);
		return *this;
	}

	void VertexArray::SetVertices(const void* vertices_data, size_t vertices_count, size_t vertex_size, const std::vector<VertexAttribDesc>& format)
	{
		if (vao == 0) {
			glGenVertexArrays(1, &vao);
		}
		glBindVertexArray(vao);

		if (vbo == 0) {
			glGenBuffers(1, &vbo);
		}
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, vertices_count * vertex_size, vertices_data, GL_STATIC_DRAW);

		for (const auto& attr : format)
		{
			glEnableVertexAttribArray(attr.location);
			glVertexAttribPointer(attr.location, attr.size, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(vertex_size), (const void*)attr.offset);
		}

		vertices = static_cast<GLsizei>(vertices_count);
	}

	void VertexArray::Draw()
	{
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vertices);
	}
}