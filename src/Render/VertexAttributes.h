#pragma once

#include "Utils/Math.h"

namespace Expanse::Render
{
	enum VertexAttributeLocation : int
	{
		POSITION  = 0,
		COLOR	  = 1,
		TEXCOORD0 = 2,
		TEXCOORD1 = 3
	};

	/*
	* Bindings of attrib locations to input parameter names in shaders
	*/

	struct VertexAttributePair
	{
		VertexAttributeLocation location;
		const char* name; 			
	};

	constexpr VertexAttributePair VertexAttributes[] = {
		{ POSITION, "position" },
		{ COLOR, "color" },
		{ TEXCOORD0, "uv" },
		{ TEXCOORD0, "uv0" },
		{ TEXCOORD1, "uv1" },
	};

	/*
	* Structures for describing vertex format (binding attrib locations to offsets in vertex buffer)
	*/

	struct VertexAttribDesc
	{
		VertexAttributeLocation location;
		int size;
		size_t offset;
	};

	template<typename Vertex>
	inline static const std::vector<VertexAttribDesc> VertexFormat;

	#define VERTEX_FORMAT(vertex) template<> inline static const std::vector<VertexAttribDesc> VertexFormat<vertex> =
	#define VERT_ATTR(vertex, attrib, loc) { loc, sizeof(vertex::attrib) / 4, offsetof(vertex, attrib) },

	/*
	* Built-in vertex types descriptions
	*/

	struct VertexP2 {
		FPoint position;
	};

	VERTEX_FORMAT(VertexP2) {
		VERT_ATTR(VertexP2, position, POSITION)
	};

	/**********************************************************************************/

	#undef VERTEX_FORMAT
	#undef VERT_ATTR
}