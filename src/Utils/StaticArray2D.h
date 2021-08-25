#pragma once

#include <array>

#include "Utils/Math.h"

namespace Expanse
{
	template<typename Elem, int X, int Y, int W, int H>
	class StaticArray2D
	{
		static constexpr Rect rect{X, Y, W, H };
		static constexpr auto Size = static_cast<size_t>(W * H);

		static_assert(W > 0 && H > 0);
	public:
		constexpr Elem& operator[](Point pt) { return _data[PointToIndex(pt)]; }
		constexpr const Elem& operator[](Point pt) const { return _data[PointToIndex(pt)]; }

		constexpr bool IndexIsValid(Point idx) const { return Contains(rect, idx); }
		constexpr Rect GetRect() const { return rect; }
		constexpr Point Origin() const { return { X, Y }; }
		constexpr int Width() const { return W; }
		constexpr int Height() const { return H; }

		auto begin() { return _data; }
		auto end() { return _data + Size; }
		auto begin() const { return _data; }
		auto end() const { return _data + Size; }

	public:
		Elem _data[W * H];

	private:
		constexpr size_t PointToIndex(const Point pt) const {
			return static_cast<size_t>(W * (H - (pt.y - Y) - 1) + (pt.x - X));
		}
	};
}