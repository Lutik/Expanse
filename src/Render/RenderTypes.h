#pragma once

#include <variant>
#include <string>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

namespace Expanse::Render
{
	enum class PrimitiveType : uint8_t
	{
		Points,
		Lines,
		LineStrip,
		Triangles,
		TriangleStrip
	};
	enum class TextureFilterType : uint8_t
	{
		Nearest,
		Linear
	};
	enum class TextureAddressMode : uint8_t
	{
		Clamp,
		Repeat
	};

	enum class BlendMode : uint8_t
	{
		None,
		Alpha,
		Add,
		Multiply,
		ImGui // find a proper name for it?
	};

	/*
	* Different resource handles
	*/
	struct Handle
	{
		static constexpr auto InvalidIndex = std::numeric_limits<size_t>::max();

		size_t index = InvalidIndex;

		bool IsValid() const noexcept { return index != InvalidIndex; }
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