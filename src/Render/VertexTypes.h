#pragma once

#include "Utils/Math.h"

#include <vector>

namespace Expanse::Render
{
	enum class VertexElementUsage : int
	{
		POSITION  = 0,
		COLOR     = 1,
		TEXCOORD0 = 2,
		TEXCOORD1 = 3
	};

	struct VertexElementLayout
	{
		VertexElementUsage usage;
		size_t size;
		size_t offset;
	};

	struct VertexLayout
	{
		size_t vertex_size = 4;
		std::vector<VertexElementLayout> elements;
	};

	template<typename Vertex>
	inline static const VertexLayout VertexFormat;

	

#define VERTEX_LAYOUT(vertex) template<> inline static const VertexLayout VertexFormat<vertex> = { sizeof(vertex), {
#define VERTEX_ELEM(vertex, attrib, usage) { VertexElementUsage::usage, sizeof(vertex::attrib), offsetof(vertex, attrib) },
#define VERTEX_END() }};

	/*
	* Built-in vertex types descriptions
	*/

	struct VertexP2
	{
		FPoint position;
	};

	VERTEX_LAYOUT(VertexP2)
		VERTEX_ELEM(VertexP2, position, POSITION)
	VERTEX_END()


	struct VertexP2T2
	{
		FPoint position;
		FPoint uv;
	};

	VERTEX_LAYOUT(VertexP2T2)
		VERTEX_ELEM(VertexP2T2, position, POSITION)
		VERTEX_ELEM(VertexP2T2, uv, TEXCOORD0)
	VERTEX_END()



#undef VERTEX_LAYOUT
#undef VERTEX_ELEM
#undef VERTEX_END

	struct VertexData
	{
		VertexData() = default;

		template<class Vertex>
		VertexData(const std::vector<Vertex>& vec)
			: ptr(vec.data())
			, size(vec.size() * sizeof(Vertex))
		{}

		const void* ptr = nullptr;
		size_t size = 0;
	};
}