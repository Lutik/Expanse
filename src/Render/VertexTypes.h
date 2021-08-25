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
		TEXCOORD1 = 3,
		NORMAL    = 4,
	};

	struct VertexElementLayout
	{
		VertexElementUsage usage;
		size_t size;
		size_t offset;
		size_t comp_size;
		bool is_integral;
		bool is_signed;
	};

	struct VertexLayout
	{
		size_t vertex_size = 4;
		std::vector<VertexElementLayout> elements;
	};

	template<typename Vertex>
	inline static const VertexLayout VertexFormat;

	

#define VERTEX_LAYOUT(vertex) template<> inline static const VertexLayout VertexFormat<vertex> = { sizeof(vertex), {
#define VERTEX_ELEM(vertex, usage, attrib, comp_type) { VertexElementUsage::usage, sizeof(vertex::attrib), offsetof(vertex, attrib), sizeof(comp_type), std::is_integral_v<comp_type>, std::is_signed_v<comp_type> },
#define VERTEX_END() }};

	/*
	* Built-in vertex types descriptions
	*/

	struct VertexP2
	{
		FPoint position;
	};

	VERTEX_LAYOUT(VertexP2)
		VERTEX_ELEM(VertexP2, POSITION, position, float)
	VERTEX_END()


	struct VertexP2T2
	{
		FPoint position;
		FPoint uv;
	};

	VERTEX_LAYOUT(VertexP2T2)
		VERTEX_ELEM(VertexP2T2, POSITION, position, float)
		VERTEX_ELEM(VertexP2T2, TEXCOORD0, uv, float)
	VERTEX_END()



#undef VERTEX_LAYOUT
#undef VERTEX_ELEM
#undef VERTEX_END
}