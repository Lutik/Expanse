#include "pch.h"

#include "VertexArrayObject.h"
#include "Utils.h"
#include "Utils/Logger/Logger.h"

namespace Expanse::Render::GL
{
	namespace
	{
		GLenum VertexElementTypeToGL(const VertexElementLayout& elem)
		{
			if (elem.is_integral)
			{
				assert(elem.comp_size == 1 || elem.comp_size == 2 || elem.comp_size == 4);

				const GLenum signed_types[] = { GL_BYTE, GL_SHORT, 0, GL_INT };
				const GLenum unsigned_types[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, 0, GL_UNSIGNED_INT };
				return elem.is_signed ? signed_types[elem.comp_size - 1] : unsigned_types[elem.comp_size - 1];
			}
			else
			{
				assert(elem.comp_size == 4);
				return GL_FLOAT;
			}
		}
	}

	void VertexArrayManager::VertexArray::Create()
	{
		if (vao == 0) {
			glGenVertexArrays(1, &vao);
		}
	}

	void VertexArrayManager::VertexArray::Free()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ibo);
		glDeleteVertexArrays(1, &vao);
		vbo = 0;
		vao = 0;
		ibo = 0;
	}

	void VertexArrayManager::VertexArray::Draw()
	{
		glBindVertexArray(vao);

		if (ibo == 0) {
			glDrawArrays(prim_type, 0, vertex_count);
		} else {
			glDrawElements(prim_type, index_count, index_type, nullptr);
		}	
	}

	void VertexArrayManager::VertexArray::SetVertices(BufferData vertex_data, const VertexLayout& format)
	{
		if (vao == 0) return;

		if (!vertex_data.ptr) {
			glDeleteBuffers(1, &vbo);
			vbo = 0;
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
			const auto location = static_cast<GLuint>(attr.usage);
			const auto components = static_cast<GLint>(attr.size / attr.comp_size);
			const auto need_normalize = attr.is_integral ? GL_TRUE : GL_FALSE;
			const auto elem_type = VertexElementTypeToGL(attr);

			glEnableVertexAttribArray(location);
			glVertexAttribPointer(location,
				components,
				elem_type,
				need_normalize,
				static_cast<GLsizei>(format.vertex_size),
				(const void*)attr.offset);
		}

		vertex_count = static_cast<GLsizei>(vertex_data.size / format.vertex_size);
	}

	void VertexArrayManager::VertexArray::SetIndices(BufferData index_data, size_t index_size)
	{
		if (!index_data.ptr) {
			glDeleteBuffers(1, &ibo);
			ibo = 0;
			index_count = 0;
			return;
		}

		glBindVertexArray(vao);
		if (ibo == 0) {
			glGenBuffers(1, &ibo);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data.size, index_data.ptr, GL_STATIC_DRAW);

		index_count = static_cast<GLsizei>(index_data.size / index_size);

		switch (index_size) {
			case 1: index_type = GL_UNSIGNED_BYTE; break;
			case 2: index_type = GL_UNSIGNED_SHORT; break;
			case 4: index_type = GL_UNSIGNED_INT; break;
			default: Log::message("Incorrect index size for index buffer");
		}
	}

	void VertexArrayManager::VertexArray::SetPrimitiveType(PrimitiveType prim)
	{
		switch (prim) {
			case PrimitiveType::Points: prim_type = GL_POINTS; break;
			case PrimitiveType::Lines: prim_type = GL_LINES; break;
			case PrimitiveType::LineStrip: prim_type = GL_LINE_STRIP; break;
			case PrimitiveType::Triangles: prim_type = GL_TRIANGLES; break;
			case PrimitiveType::TriangleStrip: prim_type = GL_TRIANGLE_STRIP; break;
		}
	}

	VertexArrayManager::~VertexArrayManager()
	{
		for (auto& mesh : vertex_arrays)
		{
			mesh.Free();
		}
	}

	Mesh VertexArrayManager::Create()
	{		
		auto isFree = [](const VertexArray& va) { return va.vao == 0; };
		const auto index = GetFreeIndexInVector(vertex_arrays, [](const VertexArray& va) { return va.vao == 0; });

		vertex_arrays[index].Create();

		return { index };
	}

	void VertexArrayManager::Free(Mesh handle)
	{
		if (!handle.IsValid()) return;

		vertex_arrays[handle.index].Free();
	}

	void VertexArrayManager::SetVertices(Mesh handle, BufferData vertex_data, const VertexLayout& format)
	{
		if (!handle.IsValid()) return;

		vertex_arrays[handle.index].SetVertices(vertex_data, format);
	}

	void VertexArrayManager::SetIndices(Mesh handle, BufferData indices_data, size_t index_size)
	{
		if (!handle.IsValid()) return;

		vertex_arrays[handle.index].SetIndices(indices_data, index_size);
	}

	void VertexArrayManager::SetPrimitiveType(Mesh handle, PrimitiveType prim)
	{
		if (!handle.IsValid()) return;

		vertex_arrays[handle.index].SetPrimitiveType(prim);
	}

	void VertexArrayManager::Draw(Mesh handle)
	{
		if (!handle.IsValid()) return;

		vertex_arrays[handle.index].Draw();
	}
}