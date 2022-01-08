#pragma once

#include <vector>
#include <array>

namespace Expanse::Render
{
	struct BufferData
	{
		BufferData() = default;
		BufferData(const void* data, size_t sz) : ptr(data), size(sz) {}

		template<std::ranges::contiguous_range Rng>
		BufferData(const Rng& range)
			: ptr(std::ranges::data(range))
			, size(std::ranges::size(range) * sizeof(std::ranges::range_value_t<Rng>))
		{}

		const void* ptr = nullptr;
		size_t size = 0;
	};
}