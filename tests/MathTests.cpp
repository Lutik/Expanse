#include "gtest/gtest.h"

#include "Utils/Math.h"
#include "Utils/Random.h"

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
}