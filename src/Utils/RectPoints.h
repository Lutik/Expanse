#pragma once

#include "Utils/Math.h"
#include <ranges>

namespace Expanse::utils
{
	namespace details
	{
		template<typename T>
		concept RectIterOrder = std::is_default_constructible_v<T> && std::is_constructible_v<T, Rect> &&
		requires(T v, Rect rect, Point& pt) {
			{ T::start_point(rect) } -> std::convertible_to<Point>;
			{ T::end_point(rect) } -> std::convertible_to<Point>;
			{ v.advance(pt) };
		};

		class IterRectLB2RT
		{
		public:
			static Point start_point(const Rect& rect) noexcept {
				return LeftBottom(rect);
			}
			static Point end_point(const Rect& rect) noexcept {
				return { rect.x, rect.y + rect.h };
			}

			IterRectLB2RT() = default;
			IterRectLB2RT(const Rect& rect)
				: start_x(rect.x)
				, end_x(rect.x + rect.w)
			{}

			void advance(Point& point)
			{
				++point.x;
				if (point.x >= end_x) {
					point.x = start_x;
					++point.y;
				}
			}
		private:
			int start_x;
			int end_x;
		};

		class IterRectRT2LB
		{
		public:
			static Point start_point(const Rect& rect) noexcept {
				return RightTop(rect);
			}
			static Point end_point(const Rect& rect) noexcept {
				return { rect.x + rect.w - 1, rect.y - 1 };
			}

			IterRectRT2LB() = default;
			IterRectRT2LB(const Rect& rect)
				: start_x(rect.x + rect.w - 1)
				, end_x(rect.x - 1)
			{}

			void advance(Point& point)
			{
				--point.x;
				if (point.x <= end_x) {
					point.x = start_x;
					--point.y;
				}
			}
		private:
			int start_x;
			int end_x;
		};
	}

	template<details::RectIterOrder IterOrder>
	class rect_points_base : public std::ranges::view_interface<rect_points_base<IterOrder>>
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

			Iterator(const Rect& rect, Point pt)
				: point(pt)
				, iter_order(rect)
			{}

			reference operator*() const noexcept { return point; }
			pointer operator->() const noexcept { return &point; }

			Iterator& operator++() noexcept
			{
				iter_order.advance(point);
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
			IterOrder iter_order;
		};

		rect_points_base() = default;
		rect_points_base(Rect r) : rect(r) {}

		auto begin() const noexcept { return Iterator{ rect, IterOrder::start_point(rect) }; }
		auto end() const noexcept { return Iterator{ rect, IterOrder::end_point(rect) }; }

	private:
		Rect rect;
	};

	

	using rect_points_lb2rt = rect_points_base<details::IterRectLB2RT>;
	using rect_points_rt2lb = rect_points_base<details::IterRectRT2LB>;
	using rect_points = rect_points_lb2rt;
}