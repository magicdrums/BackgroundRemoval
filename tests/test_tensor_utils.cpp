#include <gtest/gtest.h>

#include "core/tensor_utils.h"

TEST(TensorUtils, ValidProduct)
{
	size_t count = 0;
	EXPECT_TRUE(tensorElementCount({1, 144, 256, 3}, count));
	EXPECT_EQ(count, 1u * 144u * 256u * 3u);
}

TEST(TensorUtils, RejectsZeroDimension)
{
	size_t count = 0;
	EXPECT_FALSE(tensorElementCount({1, 0, 256, 3}, count));
}

TEST(TensorUtils, RejectsNegativeDimension)
{
	size_t count = 0;
	EXPECT_FALSE(tensorElementCount({1, -1, 256, 3}, count));
}

TEST(TensorUtils, RejectsOverflow)
{
	size_t count = 0;
	EXPECT_FALSE(tensorElementCount({100000, 100000, 100000, 100000}, count));
}
