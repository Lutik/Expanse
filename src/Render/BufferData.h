#pragma once

#include <vector>
#include <array>

namespace Expanse::Render
{
	struct BufferData
	{
		BufferData() = default;
		BufferData(const void* data, size_t sz) : ptr(data), size(sz) {}

		template<class Elem>
		BufferData(const std::vector<Elem>& vec) : ptr(vec.data()), size(vec.size() * sizeof(Elem)) {}

		template<class Elem, auto N>
		BufferData(const std::array<Elem, N>& arr) : ptr(arr.data()), size(arr.size() * sizeof(Elem)) {}

		template<class Elem, auto N>
		BufferData(const Elem (*arr)[N]) : ptr(arr), size(N * sizeof(Elem)) {}

		const void* ptr = nullptr;
		size_t size = 0;
	};
}