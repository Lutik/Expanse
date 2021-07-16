#include "pch.h"

#include "VertexArrayObject.h"

namespace Expanse::Render::GL
{
	void VertexArrayManager::VertexArray::Create()
	{
		if (vao == 0) {
			glGenVertexArrays(1, &vao);
		}
	}

	void VertexArrayManager::VertexArray::Free()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
		vbo = 0;
		vao = 0;
	}

	void VertexArrayManager::VertexArray::Draw()
	{
		glBindVertexArray(vao);

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	}

	void VertexArrayManager::VertexArray::SetVertices(VertexData vertex_data, const VertexLayout& format)
	{
		if (vao == 0) return;

		if (!vertex_data.ptr) {
			glDeleteBuffers(1, &vbo);
			vertex_count = 0;
			return;
		}

		glBindVertexArray(vao);
		if (vbo == 0) {
			glGenBuffers(1, &vbo);
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex_data.size, vertex_data.ptr, GL_STATIC_DRAW);

		for (const auto& attr : format.elements)
		{
			const int location = static_cast<int>(attr.usage);
			glEnableVertexAttribArray(location);
			glVertexAttribPointer(location,
				static_cast<GLint>(attr.size / 4),
				GL_FLOAT,
				GL_FALSE,
				static_cast<GLsizei>(format.vertex_size),
				(const void*)attr.offset);
		}

		vertex_count = static_cast<GLsizei>(vertex_data.size / format.vertex_size);
	}



	Mesh VertexArrayManager::Create()
	{
		auto isFree = [](const VertexArray& va) { return va.vao == 0; };
		auto itr = std::find_if(vertex_arrays.begin(), vertex_arrays.end(), isFree);

		if (itr == vertex_arrays.end())
		{		
			auto& va = vertex_arrays.emplace_back();
			va.Create();
			const size_t index = vertex_arrays.size() - 1;
			return { index };
		}
		else
		{
			const size_t index = itr - vertex_arrays.begin();
			return { index };
		}
	}

	void VertexArrayManager::Free(Mesh handle)
	{
		if (!handle.Valid()) return;

		vertex_arrays[handle.index].Free();
	}

	void VertexArrayManager::SetVertices(Mesh handle, VertexData vertex_data, const VertexLayout& format)
	{
		if (!handle.Valid()) return;

		vertex_arrays[handle.index].SetVertices(vertex_data, format);
	}

	void VertexArrayManager::Draw(Mesh handle)
	{
		if (!handle.Valid()) return;

		vertex_arrays[handle.index].Draw();
	}
}