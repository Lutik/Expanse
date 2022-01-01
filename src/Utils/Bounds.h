#pragma once

#include "Math.h"

#include <ranges>

namespace Expanse::utils
{
	template<Number T>
	struct Bounds
	{
		T min_x = std::numeric_limits<T>::max();
		T max_x = std::numeric_limits<T>::min();
		T min_y = std::numeric_limits<T>::max();
		T max_y = std::numeric_limits<T>::min();

		void Add(TPoint<T> pt)
		{
			if (pt.x < min_x) min_x = pt.x;
			if (pt.x > max_x) max_x = pt.x;
			if (pt.y < min_y) min_y = pt.y;
			if (pt.y > max_y) max_y = pt.y;
		}

		void Add(TRect<T> rect)
		{
			if (rect.x < min_x) min_x = rect.x;
			if (rect.y < min_y) min_y = rect.y;

			const auto pt = RightTop(rect);
			if (pt.x > max_x) max_x = pt.x;
			if (pt.y > max_y) max_y = pt.y;
		}

		TRect<T> ToRect() const
		{
			static constexpr auto Zero = static_cast<T>(0);

			if (min_x > max_x || min_y > max_y) {
				return { Zero, Zero, Zero, Zero };
			}

			auto w = max_x - min_x;
			auto h = max_y - min_y;

			if constexpr (std::is_integral_v<T>) {
				w += 1;
				h += 1;
			}

			return { min_x, min_y, w, h };
		}
	};

	template<std::ranges::input_range R>
	auto CalcBounds(R&& range)
	{
		using NumType = decltype(std::declval<std::ranges::range_value_t<R>>().x);
		Bounds<NumType> bounds;
		for (auto&& v : range) bounds.Add(v);
		return bounds.ToRect();
	}
}