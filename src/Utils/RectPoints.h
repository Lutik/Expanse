#pragma once

#include "Utils/Math.h"
#include <ranges>

namespace Expanse::utils
{
	class rect_points : public std::ranges::view_interface<rect_points>
	{
	public:
		class Iterator
		{
		public:
			using iterator_concept = std::forward_iterator_tag;
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Point;
			using pointer = const Point*;
			using reference = const Point&;

			Iterator()  = default;

			Iterator(const Iterator& rhs) = default;
			Iterator(Iterator&& rhs) = default;
			Iterator& operator=(const Iterator& rhs) = default;
			Iterator& operator=(Iterator&& rhs) = default;

			Iterator(Point pt, int width)
				: point(pt)
				, start_x(pt.x)
				, end_x(pt.x + width)
			{}

			reference operator*() const noexcept { return point; }
			pointer operator->() const noexcept { return &point; }

			Iterator& operator++() noexcept
			{
				++point.x;
				if (point.x >= end_x) {
					point.x = start_x;
					++point.y;
				}
				return *this;
			}

			Iterator operator++(int) noexcept
			{
				Iterator tmp = *this;
				++(*this);
				return tmp;
			}

			bool operator== (const Iterator& rhs) const noexcept {
				return point == rhs.point;
			};
			bool operator!= (const Iterator& rhs) const noexcept {
				return point != rhs.point;
			};

		private:
			Point point;
			int start_x;
			int end_x;
		};

		rect_points() = default;
		rect_points(Rect r) : rect(r) {}

		auto begin() const noexcept { return Iterator{ {rect.x, rect.y}, rect.w }; }
		auto end() const noexcept { return Iterator{ {rect.x, rect.y + rect.h}, rect.w }; }

	private:
		Rect rect;
	};
}