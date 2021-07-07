#include <gtest/gtest.h>

#define EXPECT_FRECT_EQ(rect1, rect2) \
EXPECT_FLOAT_EQ(rect1.x, rect2.x);\
EXPECT_FLOAT_EQ(rect1.y, rect2.y);\
EXPECT_FLOAT_EQ(rect1.w, rect2.w);\
EXPECT_FLOAT_EQ(rect1.h, rect2.h);

#define EXPECT_FPOINT_EQ(pt1, pt2) \
EXPECT_FLOAT_EQ(pt1.x, pt2.x);\
EXPECT_FLOAT_EQ(pt1.y, pt2.y)