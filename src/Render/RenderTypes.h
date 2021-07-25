#pragma once

#include <variant>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

namespace Expanse::Render
{
	enum class PrimitiveType
	{
		Points,
		Lines,
		LineStrip,
		Triangles,
		TriangleStrip
	};
	enum class TextureFilterType
	{
		Nearest,
		Linear
	};
	enum class TextureAddressMode
	{
		Clamp,
		Repeat
	};

	/*
	* Different resource handles
	*/
	inline constexpr auto InvalidHandleIndex = std::numeric_limits<size_t>::max();
	struct Handle
	{
		size_t index = InvalidHandleIndex;
		bool IsValid() const noexcept { return index != InvalidHandleIndex; }
	};

	struct Material : public Handle {};
	struct Mesh : public Handle {};
	struct Texture : public Handle {};

	/*
	* Material parameter types
	*/
	struct NoValue {};
	using TextureName = std::string;
	using MaterialParameterValue = std::variant<NoValue, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4, Texture, TextureName>;
}