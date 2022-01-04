#include "gtest/gtest.h"

#include "Utils/Array2D.h"

namespace Expanse::Tests
{
	TEST(Array2D, CopyArrayData)
	{
		const Array2D<int> src = {
			{0,  1,  2,  3},
			{4,  5,  6,  7},
			{8,  9, 10, 11},
		};

		Array2D<int> expected = {
			{0, 0, 0},
			{0, 0, 1},
			{0, 4, 5},
		};
		expected.MoveOrigin({ -1, -1 });

		Array2D<int> dst{ Rect{-1, -1, 3, 3}, 0 };

		CopyArrayData(src, dst, Rect{ 0, 0, 2, 2 }, Point{ 0, 0 });

		EXPECT_EQ(expected, dst);
	}
}