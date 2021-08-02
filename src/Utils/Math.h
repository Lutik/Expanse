#pragma once

#include <type_traits>
#include <algorithm>

namespace Expanse
{
	template<class T>
	concept Number = std::is_arithmetic_v<T>;

	template<Number T>
	struct TPoint
	{
		T x, y;

		constexpr TPoint() = default;
		constexpr TPoint(T x_, T y_) : x(x_), y(y_) {}

		template<Number T2>
		constexpr explicit TPoint(const TPoint<T2>& pt)
			: x(static_cast<T>(pt.x))
			, y(static_cast<T>(pt.y))
		{}

		constexpr TPoint& operator+= (TPoint off) { x += off.x; y += off.y; return *this; }
		constexpr TPoint& operator-= (TPoint off) { x -= off.x; y -= off.y; return *this; }
		constexpr TPoint& operator*= (T v) { x *= v; y *= v; return *this; }
		constexpr TPoint& operator/= (T v) { x /= v; y /= v; return *this; }
	};

	template<Number T>
	constexpr TPoint<T> operator+ (TPoint<T> pt1, TPoint<T> pt2) { pt1 += pt2; return pt1; }

	template<Number T>
	constexpr TPoint<T> operator- (TPoint<T> pt1, TPoint<T> pt2) { pt1 -= pt2; return pt1; }

	template<Number T>
	constexpr TPoint<T> operator* (TPoint<T> pt, T v) { pt *= v; return pt; }

	template<Number T>
	constexpr TPoint<T> operator/ (TPoint<T> pt, T v) { pt /= v; return pt; }

	template<Number T>
	constexpr bool operator==(TPoint<T> pt1, TPoint<T> pt2) { return (pt1.x == pt2.x) && (pt1.y == pt2.y); }

	template<Number T>
	constexpr bool operator!=(TPoint<T> pt1, TPoint<T> pt2) { return !operator==(pt1, pt2); }


	using Point = TPoint<int>;
	using FPoint = TPoint<float>;


	template<Number T>
	struct TRect
	{
		T x, y, w, h;

		constexpr TRect() = default;

		constexpr TRect(T x_, T y_, T w_, T h_) : x(x_), y(y_), w(w_), h(h_) {}
		
		template<Number T2>
		constexpr explicit TRect(const TRect<T2>& rect)
			: x(static_cast<T>(rect.x))
			, y(static_cast<T>(rect.y))
			, w(static_cast<T>(rect.w))
			, h(static_cast<T>(rect.h))
		{}

		constexpr TRect& operator+= (TPoint<T> off) { x += off.x; y += off.y; return *this; }
		constexpr TRect& operator-= (TPoint<T> off) { x -= off.x; y -= off.y; return *this; }
		constexpr TRect& operator*= (T v) { x *= v; y *= v; w *= v; h *= v; return *this; }
		constexpr TRect& operator/= (T v) { x /= v; y /= v; w /= v; h /= v; return *this; }
		constexpr TRect& operator*= (TPoint<T> scale) { x *= scale.x; y *= scale.y; w *= scale.x; h *= scale.y; return *this; }
		constexpr TRect& operator/= (TPoint<T> scale) { x /= scale.y; y /= scale.y; w /= scale.x; h /= scale.y; return *this; }
	};

	template<Number T>
	constexpr TRect<T> operator+ (TRect<T> rect, TPoint<T> offset) { rect += offset; return rect; }

	template<Number T>
	constexpr TRect<T> operator- (TRect<T> rect, TPoint<T> offset) { rect -= offset; return rect; }

	template<Number T>
	constexpr TRect<T> operator* (TRect<T> rect, T scale) { rect *= scale; return rect; }

	template<Number T>
	constexpr TRect<T> operator/ (TRect<T> rect, T scale) { rect /= scale; return rect; }

	template<Number T>
	constexpr TRect<T> operator* (TRect<T> rect, TPoint<T> scale) { rect *= scale; return rect; }

	template<Number T>
	constexpr TRect<T> operator/ (TRect<T> rect, TPoint<T> scale) { rect /= scale; return rect; }

	template<Number T>
	constexpr bool operator==(const TRect<T>& rect1, const TRect<T>& rect2) {
		return (rect1.x == rect2.x) && (rect1.y == rect2.y) && (rect1.w == rect2.w) && (rect1.h == rect2.h);
	}

	template<Number T>
	constexpr bool operator!=(const TRect<T>& rect1, const TRect<T>& rect2) { return !operator==(rect1, rect2); }


	using Rect = TRect<int>;
	using FRect = TRect<float>;

	constexpr Point LeftBottom(const Rect& rect) { return { rect.x, rect.y }; }
	constexpr Point LeftTop(const Rect& rect) { return { rect.x, rect.y + rect.h - 1 }; }
	constexpr Point RightBottom(const Rect& rect) { return { rect.x + rect.w - 1, rect.y }; }
	constexpr Point RightTop(const Rect& rect) { return { rect.x + rect.w - 1, rect.y + rect.h - 1 }; }

	constexpr FPoint LeftBottom(const FRect& rect) { return { rect.x, rect.y }; }
	constexpr FPoint LeftTop(const FRect& rect) { return { rect.x, rect.y + rect.h }; }
	constexpr FPoint RightBottom(const FRect& rect) { return { rect.x + rect.w, rect.y }; }
	constexpr FPoint RightTop(const FRect& rect) { return { rect.x + rect.w, rect.y + rect.h }; }
	constexpr FPoint Center(const FRect& rect) { return { rect.x + 0.5f * rect.w, rect.y + 0.5f * rect.h }; }



	template<Number T>
	constexpr bool Contains(const TRect<T>& rect, const TPoint<T>& pt)
	{
		return (pt.x >= rect.x) && (pt.x < rect.x + rect.w) && (pt.y >= rect.y) && (pt.y < rect.y + rect.h);
	}

	template<Number T>
	constexpr bool Contains(const TRect<T>& rect, const TRect<T>& in_rect)
	{
		return Contains(rect, LeftBottom(in_rect)) && Contains(rect, RightTop(in_rect));
	}

	template<Number T>
	constexpr void Inflate(TRect<T>& rect, T dx, T dy)
	{
		rect.x -= dx;
		rect.y -= dy;
		rect.w += dx * static_cast<T>(2);
		rect.h += dy * static_cast<T>(2);
	}

	template<Number T>
	constexpr TRect<T> Inflated(TRect<T> rect, T dx, T dy)
	{
		Inflate(rect, dx, dy);
		return rect;
	}

	template<Number T>
	TRect<T> Intersection(const TRect<T>& r1, const TRect<T>& r2)
	{
		const auto x = std::max(r1.x, r2.x);
		const auto y = std::max(r1.y, r2.y);
		const auto w = std::min(r1.x + r1.w, r2.x + r2.w) - x;
		const auto h = std::min(r1.y + r1.h, r2.y + r2.h) - y;

		static constexpr auto Zero = static_cast<T>(0);
		return { x, y, std::max(Zero, w), std::max(Zero, h) };
	}
}
