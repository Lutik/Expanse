#pragma once

namespace Expanse::Render
{
	inline constexpr auto InvalidHandleIndex = std::numeric_limits<size_t>::max();
	struct Handle
	{
		size_t index = InvalidHandleIndex;
		bool IsValid() const noexcept { return index != InvalidHandleIndex; }
	};

	struct Material : public Handle {};
	struct Mesh : public Handle {};
	struct Texture : public Handle {};
}