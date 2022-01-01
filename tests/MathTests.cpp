#include "gtest/gtest.h"

#include "Utils/Math.h"
#include "Utils/Random.h"
#include "Utils/Bounds.h"

#include "TestUtils.h"

#include <array>

namespace Expanse::Tests
{
	TEST(RectIntersection, PartialIntersection)
	{
		const auto rect1 = Rect{ -2, -2, 3, 4 };
		const auto rect2 = Rect{ 0, 0, 3, 4 };

		const auto result = Intersection(rect1, rect2);
		const auto expected = Rect{ 0, 0, 1, 2 };

		EXPECT_EQ(expected, result);
	}

	TEST(RectIntersection, EqualRects)
	{
		const auto rect = Rect{ -2, 1, 5, 5 };

		const auto result = Intersection(rect, rect);
		const auto expected = rect;

		EXPECT_EQ(expected, result);
	}

	TEST(RectIntersection, NoIntersection)
	{
		const auto rect1 = Rect{ -2, -2, 1, 1 };
		const auto rect2 = Rect{ 2, 2, 1, 1 };

		const auto result = Intersection(rect1, rect2);

		EXPECT_EQ(0, result.w);
		EXPECT_EQ(0, result.h);
	}

	TEST(FRectIntersection, PartialIntersection)
	{
		const auto rect1 = FRect{ -2.0f, -2.0f, 3.0f, 4.0f };
		const auto rect2 = FRect{ 0.0f, 0.0f, 3.0f, 4.0f };

		const auto result = Intersection(rect1, rect2);
		const auto expected = FRect{ 0.0f, 0.0f, 1.0f, 2.0f };

		EXPECT_FRECT_EQ(expected, result);
	}




	TEST(RandomTests, UniformIntDistribution)
	{
		constexpr int MinBound = -1;
		constexpr int MaxBound = 2;
		constexpr uint32_t MaxRngValue = 1024u;

		std::array<uint32_t, MaxBound - MinBound + 1> counters;
		counters.fill(0);

		for (uint32_t x = 0; x < MaxRngValue; ++x)
		{
			const auto val = UniformInt(x, MinBound, MaxBound);
			ASSERT_GE(val, MinBound);
			ASSERT_LE(val, MaxBound);

			++counters[val - MinBound];
		}

		for (auto count : counters)
		{
			EXPECT_GE(count, MaxRngValue / (counters.size() + 1));
		}
	}



	TEST(BoundsTests, EmptyRectIfNoInput)
	{
		utils::Bounds<float> fbounds;
		const auto frect = fbounds.ToRect();
		EXPECT_EQ(0.0f, frect.w);
		EXPECT_EQ(0.0f, frect.h);

		utils::Bounds<int> ibounds;
		const auto irect = ibounds.ToRect();
		EXPECT_EQ(0, irect.w);
		EXPECT_EQ(0, irect.h);
	}

	TEST(BoundsTests, OnePointBoundsFloat)
	{
		utils::Bounds<float> bounds;
		bounds.Add(FPoint{3.0f, 4.0f});
		const auto rect = bounds.ToRect();
		const auto expected = FRect{ 3.0f, 4.0f, 0.0f, 0.0f };
		EXPECT_FRECT_EQ(expected, rect);
	}

	TEST(BoundsTests, OnePointBoundsInt)
	{
		utils::Bounds<int> bounds;
		bounds.Add(Point{ 3, 4 });
		const auto rect = bounds.ToRect();
		const auto expected = Rect{ 3, 4, 1, 1 };
		EXPECT_EQ(expected, rect);
	}

	TEST(BoundsTests, OneRectBoundsFloat)
	{
		const auto rect = FRect{3.0f, 4.0, 1.5f, 0.8f };
		utils::Bounds<float> bounds;
		bounds.Add(rect);
		const auto result = bounds.ToRect();
		EXPECT_FRECT_EQ(rect, result);
	}

	TEST(BoundsTests, OneRectBoundsInt)
	{
		const auto rect = Rect{ 3, 4, 2, 5 };
		utils::Bounds<int> bounds;
		bounds.Add(rect);
		const auto result = bounds.ToRect();
		EXPECT_EQ(rect, result);
	}

	TEST(BoundsTests, RangeBounds)
	{
		const std::vector<int> numbers{ 0, -1, 3, 4 };
		auto to_point = [](int n) { return Point(n, n * 2); };
		const auto pt_range = numbers | std::views::transform(to_point);

		const auto rect = utils::CalcBounds(pt_range);
		const auto expected = Rect{ -1, -2, 6, 11 };
		EXPECT_EQ(expected, rect);
	}
}