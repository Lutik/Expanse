#include "gtest/gtest.h"

#include "TestUtils.h"

#include "Game/CoordSystems.h"

namespace Expanse::Tests
{
	/*
	* Cell <-> Chunk
	*/
	TEST(CellToChunk, PositiveCoordsRT)
	{
		const int chunk_size = 4;
		const auto result = Coords::CellToChunk(Point{ 3, 3 }, chunk_size);
		const auto expected = Point{ 0, 0 };

		EXPECT_EQ(expected, result);
	}

	TEST(CellToChunk, NegativeCoordsRT)
	{
		const int chunk_size = 4;
		const auto result = Coords::CellToChunk(Point{ -5, -5 }, chunk_size);
		const auto expected = Point{ -2, -2 };

		EXPECT_EQ(expected, result);
	}

	TEST(CellToChunk, PositiveCoordsLB)
	{
		const int chunk_size = 4;
		const auto result = Coords::CellToChunk(Point{ 4, 4 }, chunk_size);
		const auto expected = Point{ 1, 1 };

		EXPECT_EQ(expected, result);
	}

	TEST(CellToChunk, NegativeCoordsLB)
	{
		const int chunk_size = 4;
		const auto result = Coords::CellToChunk(Point{ -4, -4 }, chunk_size);
		const auto expected = Point{ -1, -1 };

		EXPECT_EQ(expected, result);
	}

	/* Local <-> Cell */
	TEST(LocalToCell, NegativeCoordsLB)
	{
		const int chunk_size = 4;
		const auto result = Coords::LocalToCell(Point{0, 0}, Point{-1, -1}, chunk_size);
		const auto expected = Point{-4, -4};

		EXPECT_EQ(expected, result);
	}

	TEST(LocalToCell, NegativeCoordsRT)
	{
		const int chunk_size = 4;
		const auto result = Coords::LocalToCell(Point{ 3, 3 }, Point{ -1, -1 }, chunk_size);
		const auto expected = Point{ -1, -1 };

		EXPECT_EQ(expected, result);
	}

	/* Local <-> World */

	TEST(LocalToWorld, NegativeCoordsLB)
	{
		const int chunk_size = 4;
		const auto world_origin = Point{-2, -2};
		const auto result = Coords::LocalToWorld(FPoint{0.0f, 0.0f}, Point{-1, -1}, world_origin, chunk_size);
		const auto expected = FPoint{-2.0f, -2.0f};

		EXPECT_FPOINT_EQ(expected, result);
	}
}