#pragma once

#include "Utils/Math.h"
#include "Utils/StaticArray2D.h"
#include "Utils/Array2D.h"
#include "Utils/RectPoints.h"

namespace Expanse::Game
{
	template<typename T>
	using Neighbours = StaticArray2D<T, -1, -1, 3, 3>;

	namespace Offset
	{
		inline constexpr Point LeftUp = { -1, 1};
		inline constexpr Point Up = { 0, 1 };
		inline constexpr Point RightUp = { 1, 1 };
		inline constexpr Point Left = { -1, 0 };
		inline constexpr Point Center = { 0, 0 };
		inline constexpr Point Right = { 1, 0 };
		inline constexpr Point LeftDown = { -1, -1 };
		inline constexpr Point Down = { 0, -1 };
		inline constexpr Point RightDown  = { 1, -1 };
	}

	template<class T, class Pred = std::identity>
	auto SelectNeighbours(Point cell, const Array2D<T>& arr, Pred pred = {})
	{
		using RetType = std::remove_reference_t<std::invoke_result_t<Pred, T>>;

		Neighbours<RetType> result;
		for (Point offset : utils::rect_points(result.GetRect()))
		{
			const auto index = Clamp(cell + offset, arr.GetRect());
			result[offset] = std::invoke(pred, arr[index]);
		}
		return result;
	}
}