#pragma once

#include <variant>
#include <string>
#include <algorithm>
#include <array>

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

	/*
	* Color type
	*/

	inline constexpr uint8_t ColorComponentFloatToByte(const float c)
	{
		return static_cast<uint8_t>(std::clamp(c, 0.0f, 1.0f) * 255.0f);
	}

	struct Color
	{
		uint8_t r, g, b, a;

		constexpr Color() = default;

		constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
			: r(red)
			, g(green)
			, b(blue)
			, a(alpha)
		{}

		constexpr Color(float red, float green, float blue, float alpha)
			: r(ColorComponentFloatToByte(red))
			, g(ColorComponentFloatToByte(green))
			, b(ColorComponentFloatToByte(blue))
			, a(ColorComponentFloatToByte(alpha))
		{}

		constexpr explicit Color(const std::array<uint8_t, 4>& arr)
			: Color(arr[0], arr[1], arr[2], arr[3])
		{}

		constexpr explicit Color(const glm::vec4& col)
			: Color(col.r, col.g, col.b, col.a)
		{}
	};
}